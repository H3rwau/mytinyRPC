#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <memory>
#include <unistd.h>
#include "tinyRPC/common/log.h"
#include "tinyRPC/common/config.h"
#include "tinyRPC/net/fd_event.h"
#include "tinyRPC/net/eventloop.h"
#include "tinyRPC/net/timer_event.h"
#include "tinyRPC/net/io_thread.h"
#include "tinyRPC/net/io_thread_group.h"

void testIOThread()
{
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == socketfd)
    {
        ERRORLOG("-1 == socketfd")
        exit(0);
    }
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_port = htons(12345);
    addr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &addr.sin_addr);
    int reuse = 1; // 为1代表可以重用地址，0代表不可用
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));
    int rt = bind(socketfd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
    if (0 != rt)
    {
        ERRORLOG("bind error");
        exit(1);
    }

    rt = listen(socketfd, 100);
    if (0 != rt)
    {
        ERRORLOG("listen error");
        exit(1);
    }
    // 将socketfd封装进event
    tinyRPC::FdEvent event(socketfd);
    // 给这个event添加一个读回调。
    event.listen(tinyRPC::FdEvent::IN_EVENT, [socketfd]()
                 { sockaddr_in peer_addr;
                 socklen_t addr_len = sizeof(peer_addr);
                 memset(&peer_addr,0,sizeof(peer_addr)); 
                 int clientFd = accept(socketfd,reinterpret_cast<sockaddr*>(&peer_addr),&addr_len);
                 DEBUGLOG("success get client fd[%d],peer addr [%s,%d]",clientFd,inet_ntoa(peer_addr.sin_addr),ntohs(peer_addr.sin_port)); });

    // eventLoop->addEpollEvent(&event);
    static int timer_count = 0;
    tinyRPC::TimerEvent::s_ptr timer_event = std::make_shared<tinyRPC::TimerEvent>(1000, true, [&]()
                                                                                   { INFOLOG("trigger timer event,count =%d", timer_count++); });

    // eventLoop->addTimerEvent(timer_event);
    // eventLoop->loop();
    // tinyRPC::IOThread io_thread;
    // io_thread.getEventLoop()->addEpollEvent(&event);
    // io_thread.getEventLoop()->addTimerEvent(timer_event);
    // io_thread.start();
    // io_thread.join();
    tinyRPC::IOThreadGroup io_thread_group(2);
    tinyRPC::IOThread *io_thread1 = io_thread_group.getIOThread();
    io_thread1->getEventLoop()->addEpollEvent(&event);
    io_thread1->getEventLoop()->addTimerEvent(timer_event);
    tinyRPC::IOThread *io_thread2 = io_thread_group.getIOThread();
    io_thread2->getEventLoop()->addTimerEvent(timer_event);

    io_thread_group.start();
    io_thread_group.stop();

    return;
}

int testEventLoop()
{

    // tinyRPC::Config::setConfigPath("../conf/config.xml");
    // tinyRPC::Logger::setLogLevel(tinyRPC::Config::getInstance()->m_log_level);

    tinyRPC::EventLoop *eventLoop = new tinyRPC::EventLoop();

    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == socketfd)
    {
        ERRORLOG("-1 == socketfd")
        exit(0);
    }
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_port = htons(12345);
    addr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &addr.sin_addr);
    int reuse = 1; // 为1代表可以重用地址，0代表不可用
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));
    int rt = bind(socketfd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
    if (0 != rt)
    {
        ERRORLOG("bind error");
        exit(1);
    }

    rt = listen(socketfd, 100);
    if (0 != rt)
    {
        ERRORLOG("listen error");
        exit(1);
    }
    // 将socketfd封装进event
    tinyRPC::FdEvent event(socketfd);
    // 给这个event添加一个读回调。
    event.listen(tinyRPC::FdEvent::IN_EVENT, [socketfd]()
                 { sockaddr_in peer_addr;
                 socklen_t addr_len = sizeof(peer_addr);
                 memset(&peer_addr,0,sizeof(peer_addr)); 
                 int clientFd = accept(socketfd,reinterpret_cast<sockaddr*>(&peer_addr),&addr_len);
                 DEBUGLOG("success get client fd[%d],peer addr [%s,%d]",clientFd,inet_ntoa(peer_addr.sin_addr),ntohs(peer_addr.sin_port)); });

    eventLoop->addEpollEvent(&event);
    static int timer_count = 0;
    tinyRPC::TimerEvent::s_ptr timer_event = std::make_shared<tinyRPC::TimerEvent>(1000, true, [&]()
                                                                                   { INFOLOG("trigger timer event,count =%d", timer_count++); });
    eventLoop->addTimerEvent(timer_event);
    eventLoop->loop();
    return 0;
}

int main()
{
    tinyRPC::Config::setConfigPath("../conf/config.xml");
    tinyRPC::Logger::setLogLevel(tinyRPC::Config::getInstance()->m_log_level);
    tinyRPC::Logger::getInstance()->InitGlobalLogger(1);
    testIOThread();
    // testEventLoop();
    return 0;
}