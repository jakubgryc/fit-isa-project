import socket
import struct
import json
import os
import hashlib
import sys
import subprocess
import time
import threading
from pathlib import Path
from termcolor import colored, cprint
from functools import wraps

EXPORTER_EXEC = "../p2nprobe"
pcap_dir = Path("pcaps")

stop_thread = False
message_data = {}

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


def parse_netflow_v5_record(data):
    """Parse a NetFlow v5 flow record from the given data."""
    record_fields = struct.unpack('!IIIHHIIIIHHBBBBHHBBH', data)
    record = {
        'SrcAddr': socket.inet_ntoa(struct.pack('!I', record_fields[0])),
        'DstAddr': socket.inet_ntoa(struct.pack('!I', record_fields[1])),
        'NextHop': socket.inet_ntoa(struct.pack('!I', record_fields[2])),
        'Input': record_fields[3],
        'Output': record_fields[4],
        'Packets': record_fields[5],
        'Octets': record_fields[6],
        'First': record_fields[7],
        'Last': record_fields[8],
        'Duration:': record_fields[8] - record_fields[7],
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
    hash_input = f"{record['SrcAddr']}{record['DstAddr']}{record['SrcPort']}{record['DstPort']}{record['Packets']}{record['Octets']}{record['TCPFlags']}"
    return hashlib.md5(hash_input.encode()).hexdigest()

def save_message_as_json(message_data, test_prefix, file_index):
    """Save the message data to a sorted JSON file in the logs directory."""
    os.makedirs("logs", exist_ok=True)
    filename = f"logs/{test_prefix}_test_{file_index}.json"
    
    # Sort the dictionary by the hash keys
    sorted_message_data = {k: message_data[k] for k in sorted(message_data.keys())}

    with open(filename, 'w') as json_file:
        json.dump(sorted_message_data, json_file, indent=4)
    # print(f"Saved message to {filename}")

def collector(host="127.0.0.1", port=9995):
    """Start a UDP server that listens for NetFlow v5 data on the specified host and port."""

    global message_data
    global stop_thread
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as udp_socket:
        udp_socket.bind((host, port))
        udp_socket.settimeout(0.6)
        print(f"\nListening for NetFlow v5 packets on {host}:{port}")

        message_count = 0
        message_data = {}
        while not stop_thread:
            try:
                data, addr = udp_socket.recvfrom(4096)
                print(f"\nReceived {len(data)} bytes from {addr}")

                # Parse the header
                header = parse_netflow_v5_header(data)

                # Parse each record
                record_size = 48
                num_records = header['Count']
                records = {}
                for i in range(num_records):
                    start = 24 + i * record_size
                    end = start + record_size
                    record = parse_netflow_v5_record(data[start:end])
                    
                    # Generate a unique hash key for the record
                    record_hash = generate_record_hash(record)
                    
                    # Store the record using the hash as the key
                    records[record_hash] = record

                # Create message data with header and sorted records
                message_data.update(records)

                # Save each message as a JSON file in the logs directory
                message_count += 1
            except socket.timeout:
                continue

# if __name__ == "__main__":
#     collector()




# Directory paths
# pcap_dir = Path("pcaps")
# results_dir = Path("results")
#
# # Create results directory if it doesn't exist
# results_dir.mkdir(exist_ok=True)
#
# # List to store all test functions
# test_cases = []
#
# def test_case(func):
#     """Decorator to register a function as a test case."""
#     @wraps(func)
#     def wrapper(*args, **kwargs):
#         return func(*args, **kwargs)
#     test_cases.append(wrapper)
#     return wrapper
#
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
        [f"{EXPORTER_EXEC}", "127.0.0.1:9995", str(pcap_file)], stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )
    exporter_process.communicate()
#
def run_softflowd(pcap_file):
    """Run the softflowd exporter to send data to the collector."""
    softflowd_process = subprocess.Popen(
        ["softflowd", "-r", str(pcap_file), "-n", "127.0.0.1:9995"], stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )
    softflowd_process.communicate()
#
# # Test cases using the decorator
#
# @test_case
# def check_field_existence(results_file):
#     """Example test case that checks for existence of specific fields in JSON."""
#     with open(results_file) as f:
#         data = json.load(f)
#     assert "expected_field" in data, "Field 'expected_field' missing in output"
#
# @test_case
# def validate_data_format(results_file):
#     """Example test case that validates the format of data."""
#     with open(results_file) as f:
#         data = json.load(f)
#     for record in data:
#         assert isinstance(record.get("timestamp"), str), "Timestamp is not a string"
#
# @test_case
# def compare_record_counts(my_results_file, softflowd_results_file):
#     """Example test case that compares record counts between the two JSONs."""
#     with open(my_results_file) as f1, open(softflowd_results_file) as f2:
#         my_data = json.load(f1)
#         softflowd_data = json.load(f2)
#     assert len(my_data) == len(softflowd_data), "Record counts do not match"
#
def main():
    global stop_thread
    for pcap_file in pcap_dir.glob("*.pcap"):
        print(colored(f"\nRunnin tests with PCAP file: {pcap_file}", "green"))
        stop_thread = False
        pcap_name = pcap_file.stem

        collector_process = run_collector()
        time.sleep(0.1)

        run_exporter(pcap_file)

        time.sleep(0.1)

        stop_thread = True
        collector_process.join()
        save_message_as_json(message_data, "me", pcap_name )

        stop_thread = False

        # Run the softflowd exporter
        collector_process = run_collector()
        time.sleep(0.1)

        run_softflowd(pcap_file)

        time.sleep(0.1)

        stop_thread = True
        collector_process.join()
        save_message_as_json(message_data, "soft", pcap_name)

if __name__ == "__main__":
    main()
