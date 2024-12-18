# Síťové aplikace a správa sítí - Projekt

## PCAP NetFlow v5 exportér

## Autor: Jakub Gryc <xgrycj03>
## Datum: 2024-11-17

### Popis projektu
Jedná se o implementaci NetFlow v5 exportéru v jazyce C++, který zpracovává PCAP soubory. 
Program pakety postupně načítá a agreguje je podle specifikace Netflow v5 do toků. 
Zpracovávájí se pouze TCP toky a řeší jen aktivní a neaktivní časové limity. 
Jednotlivé toky pak následně posílá pomocí UDP zpráv na kolektor.

Projekt splňuje všechny požadavky zadání.

### Překlad
Pro překlad stačí spustit příkaz `make` v kořenovém adresáři projektu. Příkaz vytvoří spustitelný soubor `p2nprobe`.

### Spuštění
./p2nprobe \<host\>:\<port\> \<pcap_file_path\> [-a <active_timeout> -i <inactive_timeout>]

Parametry:  
    \<pcap_file_path\> - cesta k PCAP souboru  
    \<host\> - IP adresa nebo doménové jméno kolektoru  
    \<port\> - port kolektoru  
    -a <active_timeout> - aktivní časový limit (výchozí hodnota 60)  
    -i <inactive_timeout> - neaktivní časový limit (výchozí hodnota 60)  

### Adresářová struktura projektu

Makefile                 # Makefile pro sestavení projektu  
manual.pdf               # Dokumentace  
README                   # Tento soubor  

src/             # Zdrojové soubory  
├── Flow.cpp  
├── FlowCache.cpp  
├── main.cpp  
├── PcapHandler.cpp  
├── Tools.cpp  
├── UDPExporter.cpp  

include/         # Hlavičkové soubory  
├── Flow.h  
├── FlowCache.h  
├── PcapHandler.h  
├── Tools.h  
├── UDPExporter.h  

obj/         # Sestavené objektové soubory vytvořené při překladu  

tests/                      # Složka s testy  

├── client.py  
├── server.py  
├── test.py                 # Skript pro spuštění testů  
├── pcaps/                  # Testovací PCAP soubory  
│   ├── test.pcap  
│   ├── test2.pcap  
│   ├── test3.pcap  
├── logs/                   # Logy testů  
│   ├── myOut_test3.json    # Příklad výstupu testu  

docs/                       # Zdrojové soubory dokumentace  
├── doc.tex  
├── citations.bib  
├── czechiso.bst  
├── figs  
│   ├── flowchart.drawio.pdf  
│   ├── nfdump.pdf  
│   ├── logo_cz.png  


