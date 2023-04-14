from scapy.all import *
import sys

CASE = 'ALL'
pcap = ""
website = ""


def funcA(pcap_file):
    print("CASE A:")
    IPAddr = ""
    MACAddr = ""
    for item in pcap_file:
        if IPAddr == "" and "DHCP Ack" in str(item.payload):
            dhcp = item.payload["DHCP"].options
            for tup in dhcp:
                if "router" in tup:
                    IPAddr = tup[1]
        if MACAddr == "" and "ARP" in str(item.payload) and item["ARP"].op == 2:
            MACAddr = item["ARP"].hwsrc
        if IPAddr != "" and MACAddr != "":
            break
    print("IPAddr: {0}".format(IPAddr))
    print("MACAddr: {0}".format(MACAddr))


def funcB(pcap_file):
    print("CASE B:")
    IPAddr_DEST = ""
    for iter, item in enumerate(pcap_file):
        if IPAddr_DEST == "" and "DNS" in item.payload:
            fd = item["DNS"].fields
            if fd["qr"] == 1 and fd["qd"] is not None and str(fd["qd"].qname, 'utf-8')[:-1] == website:
                IPAddr_DEST = fd["an"].rdata
                break
    print("IPAddr-DEST: {0}".format(IPAddr_DEST))


def funcC(pcap_file):
    print("CASE C:")
    IPAddr_SRC = ""
    IPAddr_DEST = ""
    Port = 0
    handshake = 0
    handshake1 = []
    handshake2 = []
    handshake3 = []
    records = {}
    for iter, item in enumerate(pcap_file):
        if IPAddr_DEST == "" and "DNS" in item.payload:
            fd = item["DNS"].fields
            if fd["qr"] == 1 and fd["qd"] is not None and str(fd["qd"].qname, 'utf-8')[:-1] == website:
                IPAddr_DEST = fd["an"].rdata

        if IPAddr_DEST != "" and handshake == 0 and "TCP" in item.payload and item["TCP"].underlayer[
            "IP"].dst == IPAddr_DEST:
            IPAddr_SRC = item["TCP"].underlayer["IP"].src
            Port = item["TCP"].underlayer["TCP"].dport
            break
            # TODO: display TCP handshake, stop to check whether showing all handshakes with diff ports or not
        # if iter > 1100:
        #     break
    print(
        "IPAddr-SRC: {0}\nIPAddr-DEST: {1}\nPort-DEST: {2}\nSYN: {3}\nACK: {4}".format(IPAddr_SRC, IPAddr_DEST, Port, 1,
                                                                                       0))
    print(
        "IPAddr-SRC: {0}\nIPAddr-DEST: {1}\nPort-DEST: {2}\nSYN: {3}\nACK: {4}".format(IPAddr_DEST, IPAddr_SRC, Port, 1,
                                                                                       1))
    print(
        "IPAddr-SRC: {0}\nIPAddr-DEST: {1}\nPort-DEST: {2}\nSYN: {3}\nACK: {4}".format(IPAddr_SRC, IPAddr_DEST, Port, 0,
                                                                                       1))


if __name__ == '__main__':
    argc = len(sys.argv)
    if argc == 4:
        CASE = sys.argv[1]
        pcap = sys.argv[2]
        website = sys.argv[3]
        # print(CASE, pcap,website)
    elif argc == 3:
        pcap = sys.argv[1]
        website = sys.argv[2]
        # print(CASE, pcap, website)
    else:
        print("bad input count")
        exit(0)
    pcap_file = sniff(offline=pcap)
    if CASE in ["ALL", "A"]:
        # pcap_file = sniff(offline=pcap)
        funcA(pcap_file)
    if CASE in ["ALL", "B"]:
        # pcap_file = sniff(offline=pcap)
        funcB(pcap_file)
    if CASE in ["ALL", "C"]:
        # pcap_file = sniff(offline=pcap)
        funcC(pcap_file)
