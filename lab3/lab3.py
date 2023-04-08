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
    IPAddr_DEST = ""
    for iter, item in enumerate(pcap_file):
        if IPAddr_DEST == "" and "DNS" in item.payload:
            fd = item["DNS"].fields
            if fd["qr"] == 1 and fd["qd"] is not None and str(fd["qd"].qname, 'utf-8')[:-1] == website:
                IPAddr_DEST = fd["an"].rdata
        if IPAddr_DEST !="" and "TCP" in item.payload:
            print(iter, item["TCP"].fields)
            # TODO: display TCP handshake, stop to check whether showing all handshakes with diff ports or not
        if iter>1200:
            break


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
    # packets = rdpcap(pcap)
    # print(pcap_file)
    # it=0
    # for item in pcap_file:
    # item = pcap_file[6]
    # print(item.show())
    # print(type(item.payload["DHCP"].options.show()))
    # print(type(item.payload["DHCP"].options["router"]))
    # print("time", item.time)
    # print("fields", item.fields)
    # print("overload_fields", item.overload_fields)
    # print("fields", item.fields)
    # print("payload", item.payload)
    # print("wirelen", item.wirelen)
    # print("==============")
    # it+=1
    # if(it==10):
    #     break
# for data in packets:
#     if 'UDP' in data:
#         s = repr(data)
#         print(s)
#         print(data['UDP'].sport)
