#ifndef DNSServer_h
#define DNSServer_h
#include <WiFiUdp.h>

#define DNS_QR_QUERY 0
#define DNS_QR_RESPONSE 1
#define DNS_OPCODE_QUERY 0

enum class DNSReplyCode
{
  NoError = 0,
  FormError = 1,
  ServerFailure = 2,
  NonExistentDomain = 3,
  NotImplemented = 4,
  Refused = 5,
  YXDomain = 6,
  YXRRSet = 7,
  NXRRSet = 8
};

struct DNSHeader
{
  uint16_t ID;               // 2个字节（16bit），标识分区，客户端会解析服务器返回的DNS回复报文，获取ID值与请求报文设置的ID值做比较，如果相同，则认为是同一个DNS会话
  unsigned char RD : 1;      // 表示期望递归（递归）–这个比特位被请求设置，应答的时候使用的相同的值返回。如果设置了RD，就建议域名服务器进行递归解析，递归查询的支持是可选的
  unsigned char TC : 1;      // 表示可截断的（被截断）–用于指出报文比允许的长度还要长，导致被截断
  unsigned char AA : 1;      // 表示授权回答（权威答案）–这个比特位在应答的时候才恢复，指示应答的服务器是查询域名的授权解析服务器
  unsigned char OPCode : 4;  // 通常变量0（标准查询），其他变量1（反向查询）和2（服务器状态请求），[3,15]保留值；
  unsigned char QR : 1;      // 0表示查询报文，1表示响应报文
  unsigned char RCode : 4;   
  // RCode为应答码（响应码）-这4个比特位在应答报文中设置，代表的含义如下
  // 0，无错误
  // 1，报文格式错误
  // 2，服务器失效
  // 3，域名不存在
  // 4，域名服务器不支持查询类型
  // 5，服务器拒绝给出应答
  // 6-15，保留值，暂未使用
  unsigned char Z : 3;       // 保留值，一般设置为0 ，但是我设置为0出现错误，尝试设置为3时可正常运行
  unsigned char RA : 1;      // recursion available
  uint16_t QDCount;          // 无符号16bit整数表示报文请求段中的问题记录数
  uint16_t ANCount;          // 无符号16bit整数表示报文回答段中的回答记录数
  uint16_t NSCount;          // 无符号16bit整数表示报文授权段中的授权记录数
  uint16_t ARCount;          // 无符号16bit整数表示报文附加段中的附加记录数
};

class DNSServer
{
  public:
    DNSServer();
    void processNextRequest();
    void setErrorReplyCode(const DNSReplyCode &replyCode);
    void setTTL(const uint32_t &ttl);

    // 如果成功，则返回true；如果没有可用的套接字，则返回false
    bool start(const uint16_t &port,
              const String &domainName,
              const IPAddress &resolvedIP);
    // 停止DNS服务器
    void stop();

  private:
    WiFiUDP _udp;
    uint16_t _port;
    String _domainName;
    unsigned char _resolvedIP[4];
    int _currentPacketSize;
    unsigned char* _buffer;
    DNSHeader* _dnsHeader;
    uint32_t _ttl;
    DNSReplyCode _errorReplyCode;

    void downcaseAndRemoveWwwPrefix(String &domainName);
    String getDomainNameWithoutWwwPrefix();
    bool requestIncludesOnlyOneQuestion();
    void replyWithIP();
    void replyWithCustomCode();
};
#endif

