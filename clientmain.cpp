#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include "calcLib.h"

#define MAXBUF 1024
#define TIMEOUT_SEC 5

// Enable DEBUG for verbose output
// #define DEBUG

#ifdef DEBUG
#define LOG(x) std::cerr << x << std::endl
#else
#define LOG(x)
#endif

// Parse <host>:<port> or [IPv6]:<port>
bool parseHostPort(const std::string &input, std::string &host, std::string &port)
{
  if (input.empty())
    return false;

  if (input[0] == '[')
  { // IPv6: [address]:port
    auto closeBracket = input.find(']');
    if (closeBracket == std::string::npos || closeBracket + 1 >= input.size() || input[closeBracket + 1] != ':')
      return false;
    host = input.substr(1, closeBracket - 1);
    port = input.substr(closeBracket + 2);
  }
  else
  { // IPv4 or DNS: host:port
    auto colonPos = input.rfind(':');
    if (colonPos == std::string::npos)
      return false;
    host = input.substr(0, colonPos);
    port = input.substr(colonPos + 1);
  }
  return !(host.empty() || port.empty());
}

// Read one line with timeout
std::string readLine(int sockfd)
{
  std::string line;
  char c;
  fd_set fds;
  struct timeval tv;
  tv.tv_sec = TIMEOUT_SEC;
  tv.tv_usec = 0;

  while (true)
  {
    FD_ZERO(&fds);
    FD_SET(sockfd, &fds);
    int ret = select(sockfd + 1, &fds, nullptr, nullptr, &tv);
    if (ret <= 0)
      return ""; // timeout or error
    if (recv(sockfd, &c, 1, 0) <= 0)
      return "";
    line += c;
    if (c == '\n')
      break;
  }
  return line;
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage: " << argv[0] << " <DNS|IPv4|[IPv6]:PORT>" << std::endl;
    return 1;
  }

  std::string host, port;
  if (!parseHostPort(argv[1], host, port))
  {
    std::cerr << "Invalid input format. Use <DNS|IPv4|[IPv6]:PORT>" << std::endl;
    return 1;
  }

  std::cout << "Host " << host << ", and port " << port << "." << std::endl;

  initCalcLib(); // required by assignment

  // Resolve host
  struct addrinfo hints{}, *res;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  int status = getaddrinfo(host.c_str(), port.c_str(), &hints, &res);
  if (status != 0)
  {
    std::cerr << "ERROR: RESOLVE ISSUE" << std::endl;
    return 1;
  }

  int sockfd = -1;
  for (auto p = res; p; p = p->ai_next)
  {
    sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sockfd < 0)
      continue;
    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == 0)
    {
      char addr[INET6_ADDRSTRLEN];
      void *addrPtr = (p->ai_family == AF_INET)
                          ? (void *)&((struct sockaddr_in *)p->ai_addr)->sin_addr
                          : (void *)&((struct sockaddr_in6 *)p->ai_addr)->sin6_addr;
      inet_ntop(p->ai_family, addrPtr, addr, sizeof(addr));
      LOG("Connected to " << addr << ":" << port);
      break;
    }
    close(sockfd);
    sockfd = -1;
  }
  freeaddrinfo(res);

  if (sockfd < 0)
  {
    std::cerr << "ERROR: CANT CONNECT TO " << host << std::endl;
    return 1;
  }

  // Check protocol version: only read first line to detect wrong server (fast-abort)
  std::string protoLine = readLine(sockfd);
  if (protoLine.empty() || protoLine != "TEXT TCP 1.0\n")
  {
    std::cout << "ERROR" << std::endl;
    close(sockfd);
    return 1;
  }

  // Drain remaining protocol lines until empty line
  std::string line;
  while ((line = readLine(sockfd)) != "\n" && !line.empty())
    ;

  send(sockfd, "OK\n", 3, 0);

  // Receive operation
  std::string assignment = readLine(sockfd);
  if (assignment.empty())
  {
    std::cerr << "ERROR: NO OPERATION RECEIVED" << std::endl;
    close(sockfd);
    return 1;
  }
  std::cout << "ASSIGNMENT: " << assignment;

  char op[10];
  double v1, v2;
  if (sscanf(assignment.c_str(), "%s %lf %lf", op, &v1, &v2) != 3)
  {
    std::cerr << "ERROR: INVALID FORMAT" << std::endl;
    close(sockfd);
    return 1;
  }

  double fresult = 0.0;
  int iresult = 0;
  bool isFloat = false;

  if (strcmp(op, "add") == 0)
    iresult = add((int)v1, (int)v2);
  else if (strcmp(op, "sub") == 0)
    iresult = sub((int)v1, (int)v2);
  else if (strcmp(op, "mul") == 0)
    iresult = mul((int)v1, (int)v2);
  else if (strcmp(op, "div") == 0)
    iresult = intDiv((int)v1, (int)v2);
  else if (strcmp(op, "fadd") == 0)
  {
    fresult = floatAdd(v1, v2);
    isFloat = true;
  }
  else if (strcmp(op, "fsub") == 0)
  {
    fresult = floatSub(v1, v2);
    isFloat = true;
  }
  else if (strcmp(op, "fmul") == 0)
  {
    fresult = floatMul(v1, v2);
    isFloat = true;
  }
  else if (strcmp(op, "fdiv") == 0)
  {
    fresult = floatDiv(v1, v2);
    isFloat = true;
  }
  else
  {
    std::cerr << "ERROR: UNKNOWN OPERATION" << std::endl;
    close(sockfd);
    return 1;
  }

  char resultBuf[64];
  if (isFloat)
    snprintf(resultBuf, sizeof(resultBuf), "%8.8g\n", fresult);
  else
    snprintf(resultBuf, sizeof(resultBuf), "%d\n", iresult);

  std::string trimmedResult = resultBuf;
  trimmedResult.erase(trimmedResult.find_last_not_of("\n\r") + 1);

  LOG("Calculated the result to " << trimmedResult);

  send(sockfd, resultBuf, strlen(resultBuf), 0);

  // Read server response
  std::string serverReply = readLine(sockfd);
  if (serverReply.empty())
  {
    std::cerr << "ERROR: NO SERVER RESPONSE" << std::endl;
    close(sockfd);
    return 1;
  }
  serverReply.erase(serverReply.find_last_not_of("\n\r") + 1);

  std::cout << serverReply << " (myresult=" << trimmedResult << ")" << std::endl;

  close(sockfd);
  return (serverReply == "OK") ? 0 : 1;
}
