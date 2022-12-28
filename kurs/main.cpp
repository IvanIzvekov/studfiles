#include <SFML/Graphics.hpp>
#include <iostream>
#include <pcap.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>



/* default snap length (maximum bytes per packet to capture) */
#define SNAP_LEN 1518

/* ethernet headers are always exactly 14 bytes [1] */
#define SIZE_ETHERNET 14

/* Ethernet addresses are 6 bytes */
#define ETHER_ADDR_LEN 6

#define NUMBER_OF_REQUESTS 10

using namespace std;

vector<vector<string>> globalList;
int globalCount = 0;

/* Ethernet header */
struct sniff_ethernet {
        u_char  ether_dhost[ETHER_ADDR_LEN];    /* destination host address */
        u_char  ether_shost[ETHER_ADDR_LEN];    /* source host address */
        u_short ether_type;                     /* IP? ARP? RARP? etc */
};

/* IP header */
struct sniff_ip {
        u_char  ip_vhl;                 /* version << 4 | header length >> 2 */
        u_char  ip_tos;                 /* type of service */
        u_short ip_len;                 /* total length */
        u_short ip_id;                  /* identification */
        u_short ip_off;                 /* fragment offset field */
        #define IP_RF 0x8000            /* reserved fragment flag */
        #define IP_DF 0x4000            /* dont fragment flag */
        #define IP_MF 0x2000            /* more fragments flag */
        #define IP_OFFMASK 0x1fff       /* mask for fragmenting bits */
        u_char  ip_ttl;                 /* time to live */
        u_char  ip_p;                   /* protocol */
        u_short ip_sum;                 /* checksum */
        struct  in_addr ip_src,ip_dst;  /* source and dest address */
};
#define IP_HL(ip)               (((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)                (((ip)->ip_vhl) >> 4)

/* TCP header */
typedef u_int tcp_seq;

struct sniff_tcp {
        u_short th_sport;               /* source port */
        u_short th_dport;               /* destination port */
        tcp_seq th_seq;                 /* sequence number */
        tcp_seq th_ack;                 /* acknowledgement number */
        u_char  th_offx2;               /* data offset, rsvd */
#define TH_OFF(th)      (((th)->th_offx2 & 0xf0) >> 4)
        u_char  th_flags;
        #define TH_FIN  0x01
        #define TH_SYN  0x02
        #define TH_RST  0x04
        #define TH_PUSH 0x08
        #define TH_ACK  0x10
        #define TH_URG  0x20
        #define TH_ECE  0x40
        #define TH_CWR  0x80
        #define TH_FLAGS        (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
        u_short th_win;                 /* window */
        u_short th_sum;                 /* checksum */
        u_short th_urp;                 /* urgent pointer */
};

void
got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);

string
print_payload(const u_char *payload, int len);

string
print_hex_ascii_line(const u_char *payload, int len, int offset);



/*
 * print data in rows of 16 bytes: offset   hex   ascii
 *
 * 00000   47 45 54 20 2f 20 48 54  54 50 2f 31 2e 31 0d 0a   GET / HTTP/1.1..
 */
string
print_hex_ascii_line(const u_char *payload, int len, int offset)
{
 string symb = "";
 int i;
 int gap;
 const u_char *ch;

 /* offset */
 symb.push_back(offset);
 //printf("%05d   ", offset);
 
 /* hex */
 ch = payload;
 for(i = 0; i < len; i++) {
    symb.push_back(*ch);
  //printf("%02x ", *ch);
  ch++;
  /* print extra space after 8th byte for visual aid */
  if (i == 7){
      symb.push_back(' ');
  }
  
   //printf(" ");
 }
 /* print space to handle line less than 8 bytes */
 if (len < 8)
 symb.push_back(' ');
  //printf(" ");
 
 /* fill hex gap with spaces if not full line */
 if (len < 16) {
  gap = 16 - len;
  for (i = 0; i < gap; i++) {
      symb.push_back('  ');
   //printf("   ");
  }
 }
 //printf("   ");
 
 /* ascii (if printable) */
 ch = payload;
 for(i = 0; i < len; i++) {
  if (isprint(*ch))
  symb.push_back(*ch);
   //printf("%c", *ch);
  else
  symb.push_back('.');
   //printf(".");
  ch++;
 }
symb.push_back('\n');
 //printf("\n");

return symb;
}

/*
 * print packet payload data (avoid printing binary data)
 */
string
print_payload(const u_char *payload, int len)
{
 string content;
 int len_rem = len;
 int line_width = 16;   /* number of bytes per line */
 int line_len;
 int offset = 0;     /* zero-based offset counter */
 const u_char *ch = payload;

 if (len <= 0)
  return " ";

 /* data fits on one line */
 if (len <= line_width) {
  content.append(print_hex_ascii_line(ch, len, offset));
  return " ";
 }

 /* data spans multiple lines */
 for ( ;; ) {
  /* compute current line length */
  line_len = line_width % len_rem;
  /* print line */
  print_hex_ascii_line(ch, line_len, offset);
  /* compute total remaining */
  len_rem = len_rem - line_len;
  /* shift pointer to remaining bytes to print */
  ch = ch + line_len;
  /* add offset */
  offset = offset + line_width;
  /* check if we have line width chars or less */
  if (len_rem <= line_width) {
   /* print last line and get out */
   content.append(print_hex_ascii_line(ch, len_rem, offset));
   break;
  }
 }

return content;
}

/*
 * dissect/print packet
 */
void
got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{

 static int count = 1;                   /* packet counter */
 
 /* declare pointers to packet headers */
 const struct sniff_ethernet *ethernet;  /* The ethernet header [1] */
 const struct sniff_ip *ip;              /* The IP header */
 const struct sniff_tcp *tcp;            /* The TCP header */
 const u_char *payload;                    /* Packet payload */

 int size_ip;
 int size_tcp;
 int size_payload;
 
 printf("\nPacket number %d:\n", count);
 count++;
 
 /* define ethernet header */
 ethernet = (struct sniff_ethernet*)(packet);
 
 /* define/compute ip header offset */
 ip = (struct sniff_ip*)(packet + SIZE_ETHERNET);
 size_ip = IP_HL(ip)*4;
 if (size_ip < 20) {
  printf("   * Invalid IP header length: %u bytes\n", size_ip);
  return;
 }

 /* print source and destination IP addresses */
 printf("       From: %s\n", inet_ntoa(ip->ip_src));
 printf("         To: %s\n", inet_ntoa(ip->ip_dst));
 
 /* determine protocol */ 
 switch(ip->ip_p) {
  case IPPROTO_TCP:
   printf("   Protocol: TCP\n");
   break;
  case IPPROTO_UDP:
   printf("   Protocol: UDP\n");
   return;
  case IPPROTO_ICMP:
   printf("   Protocol: ICMP\n");
   return;
  case IPPROTO_IP:
   printf("   Protocol: IP\n");
   return;
  default:
   printf("   Protocol: unknown\n");
   return;
 }
 
 /*
  *  OK, this packet is TCP.
  */
 
 /* define/compute tcp header offset */
 tcp = (struct sniff_tcp*)(packet + SIZE_ETHERNET + size_ip);
 size_tcp = TH_OFF(tcp)*4;
 if (size_tcp < 20) {
  printf("   * Invalid TCP header length: %u bytes\n", size_tcp);
  return;
 }
 
 printf("   Src port: %d\n", ntohs(tcp->th_sport));
 printf("   Dst port: %d\n", ntohs(tcp->th_dport));
 
 /* define/compute tcp payload (segment) offset */
 payload = (u_char *)(packet + SIZE_ETHERNET + size_ip + size_tcp);
 
 /* compute tcp payload (segment) size */
 size_payload = ntohs(ip->ip_len) - (size_ip + size_tcp);
 
 /*
  * Print payload data; it might be binary, so don't just
  * treat it as a string.
  */
 if (size_payload > 0) {
  printf("   Payload (%d bytes):\n", size_payload);
  print_payload(payload, size_payload);
 }

return;
}




void packetHandler(u_char *userData, const struct pcap_pkthdr* pkthdr, const u_char* packet) {
    vector<string> currPacket;
    currPacket.push_back(to_string(pkthdr->len));
    cout << pkthdr->len << endl;
    const struct sniff_ip *ip;
    ip = (struct sniff_ip*)(packet + SIZE_ETHERNET);
    currPacket.push_back(string(inet_ntoa(ip->ip_src)));
    currPacket.push_back(string(inet_ntoa(ip->ip_dst)));
    printf("       From: %s\n", inet_ntoa(ip->ip_src));
    printf("         To: %s\n", inet_ntoa(ip->ip_dst));
    switch(ip->ip_p) {
    case IPPROTO_TCP:
    currPacket.push_back("TCP");
    break;
    case IPPROTO_UDP:
    currPacket.push_back("UDP");
    break;
    case IPPROTO_ICMP:
    currPacket.push_back("ICMP");
    break;
    case IPPROTO_IP:
    currPacket.push_back("IP");
    break;
    default:
    currPacket.push_back("unknown");
    break;
    }

    cout << endl;
    
    
    int size_ip;
    int size_tcp;
    int size_payload;

    size_ip = IP_HL(ip)*4;
    const struct sniff_tcp *tcp;
    tcp = (struct sniff_tcp*)(packet + SIZE_ETHERNET + size_ip);
    u_char *payload;
    payload = (u_char *)(packet + SIZE_ETHERNET + size_ip + size_tcp);
    size_payload = ntohs(ip->ip_len) - (size_ip + size_tcp);
    string content = print_payload(payload, size_payload);

    //char *payloadChar = reinterpret_cast<char *>(payload);
    cout << payload << endl;
    currPacket.push_back(content);
    globalList.push_back(currPacket);
}

pcap_t *handle;

void getRequests() {
    char errbuf[PCAP_ERRBUF_SIZE];  /* error buffer */
    char *dev = pcap_lookupdev(errbuf);   /* capture device name */
    

    char filter_exp[] = "";  /* filter expression */
    struct bpf_program fp;   /* compiled filter program (expression) */
    bpf_u_int32 mask;   /* subnet mask */
    bpf_u_int32 net;   /* ip */
    int num_packets = 10;   /* number of packets to capture */

    if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1) {
    fprintf(stderr, "Couldn't get netmask for device %s: %s\n",
        dev, errbuf);
    net = 0;
    mask = 0;
    }

    /* print capture info */
    printf("Device: %s\n", dev);
    printf("Number of packets: %d\n", num_packets);
    printf("Filter expression: %s\n", filter_exp);

    /* open capture device */
    handle = pcap_open_live(dev, SNAP_LEN, 1, 1000, errbuf);
    if (handle == NULL) {
    fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
    exit(EXIT_FAILURE);
    }

    /* make sure we're capturing on an Ethernet device [2] */
    if (pcap_datalink(handle) != DLT_EN10MB) {
    fprintf(stderr, "%s is not an Ethernet\n", dev);
    exit(EXIT_FAILURE);
    }

    /* compile the filter expression */
    if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
    fprintf(stderr, "Couldn't parse filter %s: %s\n",
        filter_exp, pcap_geterr(handle));
    exit(EXIT_FAILURE);
    }

    /* apply the compiled filter */
    if (pcap_setfilter(handle, &fp) == -1) {
    fprintf(stderr, "Couldn't install filter %s: %s\n",
        filter_exp, pcap_geterr(handle));
    exit(EXIT_FAILURE);
    }

    /* now we can set our callback function */
    pcap_loop(handle, num_packets, packetHandler, NULL);


    printf("\nCapture complete.\n");

}

void startView(){
    sf::RenderWindow window(sf::VideoMode(1000, 500), "Izvekov Ivan, Kostin Andrey IP017");
    sf::Font font;
    font.loadFromFile("Roboto.ttf");
    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(14);
    text.setFillColor(sf::Color::Red);
    while (window.isOpen())
    {
        //window.clear(sf::Color::White);
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::KeyPressed){
                if(event.key.code == 72) {
                    if(globalCount < 10) {
                        globalCount += 10;
                    }
                }
                if(event.key.code == 71) {
                    if(globalCount > 0) {
                        globalCount -= 10;
                    }
                }
                if(event.key.code == 17) {
                    cout << 212;
                    globalCount = 0;
                    globalList.clear();
                    cout << 1;
                    pcap_loop(handle, 10, packetHandler, NULL);
                    
                }
            }
            if (event.type == sf::Event::Closed)
                window.close();
            
        }

        window.clear(sf::Color::White);

        text.setString("Number");
        text.setPosition(0, 0);
        window.draw(text);

        text.setString("Length");
        text.setPosition(150, 0);
        window.draw(text);

        text.setString("From");
        text.setPosition(300, 0);
        window.draw(text);

        text.setString("To");
        text.setPosition(450, 0);
        window.draw(text);

        text.setString("Protocol");
        text.setPosition(600, 0);
        window.draw(text);

        text.setString("Content");
        text.setPosition(750, 0);
        window.draw(text);

        for (int i = globalCount; i < globalCount + 10; i++) {
            text.setString(to_string(i + 1));
            text.setPosition(0, (i % 10 + 1) * 40);
            window.draw(text);
            for (int j = 0; j < 5; j++) {
                text.setString(globalList[i][j]);
                text.setPosition((j + 1) * 150, ((i % 10) + 1) * 40);
                window.draw(text);
                
            }
        }
        
        
        window.display();
    }
}

int main() {    
    getRequests();
    startView();

    return 0;
}
