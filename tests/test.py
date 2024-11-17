# Author: Jakub Gryc <xgrycj03>
# Brief: Simple test program to test the Netflow v5 exporter

import socket
import struct
import json
import os
import hashlib
import sys
import subprocess
import time
import threading
# import numpy as np
from datetime import datetime
from zoneinfo import ZoneInfo
from pathlib import Path
from termcolor import colored, cprint
from functools import wraps

CREATE_JSON = False
RUN_TESTS = False
RUN_SOFTFLOWD = False
RUN_DURATION_TEST = False


EXPORTER_EXEC = "../p2nprobe"
PCAP_DIR = Path("pcaps")
ACTIVE = 60
INACTIVE = 60

stop_thread = False
message_data = {}
test_cases = []
total_success = 0

def print_help():
    print(
    """
Usage: python3 test.py [OPTIONS]
[OPTIONS]:
-c               Create JSON files with the exported data
-d | --duration  Run duration tests to test active duration of your flows
-t | --test      Run tests on the exported data with softflowd (not reliable on bigger pcaps)
--softflowd      Create JSON outputs with softflowd
-a [ACTIVE]      Set active timeout in seconds
-i [INACTIVE]    Set inactive timeout in seconds
    """)


def test_case(func):
    """Decorator to register a function as a test case."""
    @wraps(func)
    def wrapper(*args, **kwargs):
        return func(*args, **kwargs)
    test_cases.append(wrapper)
    return wrapper

def parse_netflow_v5_header(data):
    """Parse the NetFlow v5 header from the given data."""
    header_fields = struct.unpack('!HHIIIIBBH', data[:24])
    header = {
        'Version': header_fields[0],
        'Count': header_fields[1],
        'SysUptime': header_fields[2],
        'UnixSecs': header_fields[3],
        'UnixNsecs': header_fields[4],
        'FlowSequence': header_fields[5],
        'EngineType': header_fields[6],
        'EngineID': header_fields[7],
        'SamplingInterval': header_fields[8]
    }
    return header


def parse_netflow_v5_record(data, timestamp):
    """Parse a NetFlow v5 flow record from the given data."""
    timezone = ZoneInfo("Europe/Prague")
    record_fields = struct.unpack('!IIIHHIIIIHHBBBBHHBBH', data)

    max32 = 2**32-1
    first = record_fields[7]
    timestamp_seconds = (timestamp - (max32 - first)) / 1000

    dt_object = datetime.fromtimestamp(timestamp_seconds, tz=timezone)
    formatted_time = dt_object.strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]
    record = {
        'Timestamp': formatted_time,
        'SrcAddr': socket.inet_ntoa(struct.pack('!I', record_fields[0])),
        'DstAddr': socket.inet_ntoa(struct.pack('!I', record_fields[1])),
        'NextHop': socket.inet_ntoa(struct.pack('!I', record_fields[2])),
        'Input': record_fields[3],
        'Output': record_fields[4],
        'Packets': record_fields[5],
        'Octets': record_fields[6],
        'First': record_fields[7],
        'Last': record_fields[8],
        'Duration': record_fields[8] - record_fields[7],
        'SrcPort': record_fields[9],
        'DstPort': record_fields[10],
        'Padding': record_fields[11],
        'TCPFlags': record_fields[12],
        'Protocol': record_fields[13],
        'Tos': record_fields[14],
        'SrcAS': record_fields[15],
        'DstAS': record_fields[16],
        'SrcMask': record_fields[17],
        'DstMask': record_fields[18],
        'Padding2': record_fields[19],
    }
    return record

def generate_record_hash(record):
    """Generate a hash key for a record based on specific fields."""
    hash_input = f"{record['SrcAddr']}{record['DstAddr']}{record['SrcPort']}{record['DstPort']}"
    return hashlib.md5(hash_input.encode()).hexdigest()

def save_message_as_json(message_data, test_prefix, file_index):
    """Save the message data to a sorted JSON file in the logs directory."""
    os.makedirs("logs", exist_ok=True)
    filename = f"logs/{test_prefix}_{file_index}.json"
    
    # Sort the dictionary by the hash keys
    headers = message_data["headers"]
    records = message_data["records"]

    sorted_records = {key: records[key] for key in sorted(records.keys())}

    sorted_message_data = { "headers": headers, "records": sorted_records }

    with open(filename, 'w') as json_file:
        json.dump(sorted_message_data, json_file, indent=4)
    # print(f"Saved message to {filename}")

def collector(host="127.0.0.1", port=9995):
    """Start a UDP server that listens for NetFlow v5 data on the specified host and port."""

    global message_data
    global stop_thread
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as udp_socket:
        udp_socket.bind((host, port))
        udp_socket.settimeout(6.5)
        # print(f"\nListening for NetFlow v5 packets on {host}:{port}")

        message_count = 0
        message_data = { "headers": {}, "records": {} }
        header_index = 0
        same_hash_idx = 1
        flow_count = 0
        while not stop_thread:
            try:
                data, addr = udp_socket.recvfrom(4096)
                # print(f"\nReceived {len(data)} bytes from {addr}")

                # Parse the header
                header = parse_netflow_v5_header(data)
                timestamp = header['UnixSecs']*1000 - header['SysUptime']+ header['UnixNsecs'] // 1000000
                flow_count += header['Count']
                header_dict = {f"header_{header_index}": header}


                # Parse each record
                record_size = 48
                num_records = header['Count']
                records = {}
                for i in range(num_records):
                    start = 24 + i * record_size
                    end = start + record_size
                    record = parse_netflow_v5_record(data[start:end], timestamp)
                    
                    # Generate a unique hash key for the record
                    record_hash = generate_record_hash(record)
                    
                    # Store the record using the hash as the key

                    if record_hash in message_data["records"].keys():
                        count = sum([1 for key in message_data["records"].keys() if key == record_hash or key.startswith(record_hash+"_")])
                        record_hash =  record_hash + f"_{count}"
                    records[record_hash] = record

                # Create message data with header and sorted records
                message_data["headers"].update(header_dict)
                message_data["records"].update(records)

                # Save each message as a JSON file in the logs directory
                message_count += 1
                header_index += 1
            except socket.timeout:
                continue
        print(f"\nTotal flows: {flow_count}")



def log_success(message):
    """Log a success message to the console."""
    cprint(message, "green")

def log_failure(message):
    """Log a failure message to the console."""
    cprint(message, "red")

def log_warning(message):
    """Log a warning message to the console."""
    cprint(message, "yellow")



def run_collector():
    """Run the collector and write output to a JSON file."""
    collector_thread = threading.Thread(target=collector)
    # collector_thread.daemon = True
    collector_thread.start()
    return collector_thread
#
def run_exporter(pcap_file):
    """Run your exporter to send data to the collector."""
    exporter_process = subprocess.Popen(
        [f"{EXPORTER_EXEC}", "127.0.0.1:9995", str(pcap_file), "-a" , f"{ACTIVE}", "-i", f"{INACTIVE}"], stdout=subprocess.PIPE, stderr=subprocess.PIPE

    )
    exporter_process.communicate()
#
def run_softflowd(pcap_file):
    """Run the softflowd exporter to send data to the collector."""
    softflowd_process = subprocess.Popen(
        ["softflowd", "-r", str(pcap_file), "-n", "127.0.0.1:9995" ], stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )
    softflowd_process.communicate()
#
# # Test cases using the decorator
#
@test_case
def check_hashes(my_result_file, softflowd_result_file):
    global total_success
    with open(my_result_file) as f1, open(softflowd_result_file) as f2:
        my_data = json.load(f1)
        softflowd_data = json.load(f2)

    if len(my_data["records"]) != len(softflowd_data["records"]):
        log_failure("Different amount of flows in the two files")
        log_failure(f"My amount: {len(my_data['records'])}, softflowd amount: {len(softflowd_data['records'])}")
        return
    for record in my_data["records"]:
        if record not in softflowd_data["records"]:
            log_failure(f"Record with hash {record} not found in Softflowd data")
            return
    log_success("SUCCESS")
    total_success += 1


@test_case
def check_packet_amount(my_result_file, softflowd_result_file):
    global total_success
    with open(my_result_file) as f1, open(softflowd_result_file) as f2:
        my_data = json.load(f1)
        softflowd_data = json.load(f2)

    if len(my_data["records"]) != len(softflowd_data["records"]):
        log_failure("Different amount of flows in the two files")
        return

    my_packet_amount = [record['Packets'] for record in my_data["records"].values()]
    softflowd_packet_amount = [record['Packets'] for record in softflowd_data["records"].values()]
    for i, (my_packet, softflowd_packet) in enumerate(zip(my_packet_amount, softflowd_packet_amount)):
        if my_packet != softflowd_packet:
            log_failure(f"Packet amounts do not match at index {i}")
            log_failure(f"My packet amount: {my_packet}, Softflowd packet amount: {softflowd_packet}")
            return
    log_success("SUCCESS")
    total_success += 1


@test_case
def check_octet_amount(my_result_file, softflowd_result_file):
    global total_success
    with open(my_result_file) as f1, open(softflowd_result_file) as f2:
        my_data = json.load(f1)
        softflowd_data = json.load(f2)

    if len(my_data["records"]) != len(softflowd_data["records"]):
        log_failure("Different amount of flows in the two files")
        return

    my_octet_amount = [record['Octets'] for record in my_data["records"].values()]
    softflowd_octet_amount = [record['Octets'] for record in softflowd_data["records"].values()]
    for i, (my_packet, softflowd_packet) in enumerate(zip(my_octet_amount, softflowd_octet_amount)):
        if my_packet != softflowd_packet:
            log_failure(f"Octet amounts do not match at index {i}")
            log_failure(f"My octet amount: {my_packet}, Softflowd octet amount: {softflowd_packet}")
            return
    log_success("SUCCESS")
    total_success += 1

@test_case
def check_header_amount(my_result_file, softflowd_result_file):
    global total_success
    with open(my_result_file) as f1, open(softflowd_result_file) as f2:
        my_data = json.load(f1)
        softflowd_data = json.load(f2)

    if len(my_data["headers"]) != len(softflowd_data["headers"]):
        log_failure("Different total amount of headers (messages) sent")
        log_failure(f"My amount: {len(my_data['headers'])}, softflowd amount: {len(softflowd_data['headers'])}")
        return
    log_success("SUCCESS")
    total_success += 1

@test_case
def check_single_flow_duration(my_result_file, softflowd_result_file):
    global total_success
    with open(my_result_file) as f1, open(softflowd_result_file) as f2:
        my_data = json.load(f1)
        softflowd_data = json.load(f2)

    if len(my_data["records"]) != len(softflowd_data["records"]):
        log_failure("Different amount of flows in the two files")
        return

    my_duration = [record['Duration'] for record in my_data["records"].values() if record['Packets'] != 1]
    softflowd_duration = [record['Duration'] for record in softflowd_data["records"].values() if record['Packets'] != 1]
    err = 0
    for i, (my_packet, softflowd_packet) in enumerate(zip(my_duration, softflowd_duration)):
        if my_packet != softflowd_packet:
            err += 1
            # log_failure(f"Duration does not match at index {i}")
            # log_failure(f"My duration amount: {my_packet}, Softflowd duration amount: {softflowd_packet}")
            # return
    if err == 0:
        log_success("SUCCESS")
        total_success += 1
    else:
        log_failure(f"Errors: {err}")


@test_case
def check_tcp_flags(my_result_file, softflowd_result_file):
    global total_success
    with open(my_result_file) as f1, open(softflowd_result_file) as f2:
        my_data = json.load(f1)
        softflowd_data = json.load(f2)

    if len(my_data["records"]) != len(softflowd_data["records"]):
        log_failure("Different amount of flows in the two files")
        return

    my_tcp_flags = [record['TCPFlags'] for record in my_data["records"].values()]
    softflowd_tcp_flags = [record['TCPFlags'] for record in softflowd_data["records"].values()]
    for i, (my_packet, softflowd_packet) in enumerate(zip(my_tcp_flags, softflowd_tcp_flags)):
        if my_packet != softflowd_packet:
            log_failure(f"Octet amounts do not match at index {i}")
            log_failure(f"My octet amount: {my_packet}, Softflowd octet amount: {softflowd_packet}")
            return
    log_success("SUCCESS")
    total_success += 1



@test_case
def check_flow_sequence_in_header(my_result_file, softflowd_result_file):
    global total_success
    with open(my_result_file) as f1, open(softflowd_result_file) as f2:
        my_data = json.load(f1)
        softflowd_data = json.load(f2)

    if len(my_data["headers"]) != len(softflowd_data["headers"]):
        log_failure("Different total amount of headers (messages) sent")
        log_failure(f"My amount: {len(my_data['headers'])}, softflowd amount: {len(softflowd_data['headers'])}")
        return
    my_flow_sequence = [header['FlowSequence'] for header in my_data["headers"].values()]
    softflowd_flow_sequence = [header['FlowSequence'] for header in softflowd_data["headers"].values()]

    for i, (my_packet, softflowd_packet) in enumerate(zip(my_flow_sequence, softflowd_flow_sequence)):
        if my_packet != softflowd_packet:
            log_failure(f"Flow sequence does not match at index {i}")
            log_failure(f"My flow sequence: {my_packet}, Softflowd flow sequence: {softflowd_packet}")
            return
    total_success += 1
    log_success("SUCCESS")


def check_max_duration(my_result_file):
    global total_success
    with open(my_result_file) as f1:
        my_data = json.load(f1)


    my_duration = [record for record in my_data["records"].items()]
    for i, (hash, my_packet) in enumerate(my_duration):
        duration = my_packet['Duration']
        if duration > ACTIVE * 1000:
            log_failure(f"\nhash: {hash}")
            log_failure(f"Duration is greater than active timer duration at index {i}")
            continue
        elif duration == ACTIVE * 1000:
            ## Not really an error, when rounding to milliseconds the duration can be equal to the active timer
            ## Hard to test, use wireshark with the source PCAP to verify the actual duration
            ## example:
            ## active timeout: 10s
            ## First packet:             2024-10-31 15:00:00.400000
            ## Last good packet:         2024-10-31 15:00:10.3999000
            ## Another packet (expired): 2024-10-31 15:00:10.400020 (expired by 20us)

            ## depending on your implementation, when rounding to milliseconds the duration of the flow
            ## would probably be still 10000ms
            log_warning(f"\nhash: {hash}")
            log_warning(f"Duration is equal to active timer duration at index {i}")
    log_success("SUCCESS")
    total_success += 1



def create_output():
    global stop_thread
    global RUN_SOFTFLOWD
    global message_data
     
    for pcap_file in PCAP_DIR.glob("*.pcap"):
        print("-" * 30)
        print(colored(f"\nPCAP file: {pcap_file}", "cyan"))
        print(colored(f"\nExporting data with my exporter", "yellow"))
        stop_thread = False
        pcap_name = pcap_file.stem

        collector_process = run_collector()
        time.sleep(0.4)

        run_exporter(pcap_file)

        time.sleep(0.4)

        stop_thread = True
        collector_process.join()
        save_message_as_json(message_data, "myOut", pcap_name )

        stop_thread = False

        if not RUN_SOFTFLOWD:
            continue

        # Run the softflowd exporter
        print(colored(f"\nExporting data with softlowd", "yellow"))
        collector_process = run_collector()
        time.sleep(0.1)

        run_softflowd(pcap_file)

        time.sleep(0.1)

        stop_thread = True
        collector_process.join()
        save_message_as_json(message_data, "softOut", pcap_name)

def run_tests():
    log_files = Path("logs")
    my_output = [file for file in log_files.glob("*.json") if "myOut" in file.stem]
    soft_output = [file for file in log_files.glob("*.json") if "softOut" in file.stem]

    idx = 1
    num_of_tests = len(test_cases * len(my_output))
    
    print("\n\n")
    print("****************************************")
    print("*           RUNNING TESTS              *")
    print("****************************************")
    for test_case in test_cases:
        for me_file, soft_file in zip(sorted(my_output), sorted(soft_output)):
            print("-" * 30)
            print(colored(f"\nTEST CASE {idx}/{num_of_tests}", "cyan"))
            print(f"Test files: {me_file}, {soft_file}")

            print(colored(f"\nRunning test: {test_case.__name__}", "yellow"))
            test_case(me_file, soft_file)
            idx += 1
            # time.sleep(0.01)

    print(colored(f"\n\nTotal successful tests: {total_success}/{num_of_tests}", "light_magenta"))


def run_duration_tests():
    """
    Run duration tests on your outputs, checks if the duration of each flow is within the active timer
    """
    log_files = Path("logs")
    my_output = [file for file in log_files.glob("*.json") if "myOut" in file.stem]

    idx = 1
    num_of_tests = len(my_output)
    
    print("\n\n")
    print("****************************************")
    print("*           RUNNING TESTS              *")
    print("****************************************")
    for file in my_output:
        print("-" * 30)
        print(colored(f"\nTEST CASE {idx}/{num_of_tests}", "cyan"))
        print(f"Test files: {file}")

        print(colored(f"\nRunning test: check_max_duration", "yellow"))
        check_max_duration(file)
        idx += 1
        # time.sleep(0.01)

    print(colored(f"\n\nTotal successful tests: {total_success}/{num_of_tests}", "light_magenta"))

def main():

    create_output() if CREATE_JSON else True
    run_tests() if (RUN_TESTS and RUN_SOFTFLOWD) else True
    run_duration_tests() if RUN_DURATION_TEST else True



    

if __name__ == "__main__":

    # create outputs inside ./logs directory
    if "-c" in sys.argv:
        CREATE_JSON = True

    # run tests compared to the softflowd outputs, not really reliable
    if "-t" in sys.argv or "--test" in sys.argv:
        RUN_TESTS = True

    # run duration tests only on your outputs, checks active duration
    if "-d" in sys.argv or "--duration" in sys.argv:
        RUN_DURATION_TEST = True

    if "-h" in sys.argv or "--help" in sys.argv:
        print_help()
        exit(0)

    # set active timer
    if "-a" in sys.argv:
        ACTIVE = int(sys.argv[sys.argv.index("-a")+1])

    # set inactive timer
    if "-i" in sys.argv:
        INACTIVE = int(sys.argv[sys.argv.index("-i")+1])

    if "--softflowd" in sys.argv:
        RUN_SOFTFLOWD = True
    main()
