#include "tcpserver.h"
//#include "response.h"
#include <pthread.h>
#include <sys/epoll.h>
#include "threadpool.h"
#include "TaskQueue.h"
pthread_mutex_t m_mutex;

struct SockInfo
{
    TcpServer* s;
    TcpSocket* tcp;
    sockaddr_in addr;
};

struct PoolInfo
{
    TcpServer* s;
    TcpSocket* tcp;
    sockaddr_in addr;
    ThreadPool<SockInfo>* pool;
};

void working(void* arg)
{
    // struct SockInfo* pinfo = static_cast<struct SockInfo*>(arg);
    struct SockInfo* pinfo = (struct SockInfo*)(arg);
    // 连接建立成功, 打印客户端的IP和端口信息
    char ip[32];
    printf("客户端的IP: %s, 端口: %d\n",
        inet_ntop(AF_INET, &pinfo->addr.sin_addr.s_addr, ip, sizeof(ip)),
        ntohs(pinfo->addr.sin_port));

    // 5. 通信
    while (1)
    {
        printf("接收数据: .....\n");
        string msg = pinfo->tcp->recvMsg();
        if (!msg.empty())
        {
            cout << msg << endl << endl << endl;
        }
        else
        {
            break;
        }
    }
    delete pinfo->tcp;
    //delete pinfo;
    //return nullptr;
}

void acceptconn(void* arg)
{
    // 3. 阻塞并等待客户端的连接
    PoolInfo* pinfo = (PoolInfo*)(arg);
    while (1)
    {
        //pthread_t aid;
        //TcpSocket* tcp = s.acceptConn(&info->addr);
        //pthread_create(&aid, NULL, accepting, info);
        //pthread_detach(aid);
        pinfo->tcp = (*pinfo->s).acceptConn(&pinfo->addr);
        if (pinfo->tcp == nullptr)
        {
            cout << "重试...." << endl;
            continue;
        }
        SockInfo* info = new SockInfo;
        info->addr = pinfo->addr;
        info->s = pinfo->s;
        info->tcp = pinfo->tcp;
        
        Task<SockInfo> task(working, info);
        pinfo->pool->threadPoolAdd(task);
    } // 先不需要析构pool 线程池

}

int main()
{
    // 1. 创建监听的套接字
    TcpServer *s = new TcpServer;
    // 2. 绑定本地的IP port并设置监听
    int lfd = s->setListen(8080);

    // 创建epoll实例
    int epfd = epoll_create(1);
    if(epfd == -1)
    {
        perror("pefd");
        exit(0);
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = lfd;

    int ec = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
    if(ec == -1)
    {
        cout<<"epoll_ctl调用失败"<<endl;
    }

    struct epoll_event evs[1024];
    int size = sizeof(evs)/sizeof(evs[0]);
    
    while(1)
    {
        int num = epoll_wait(epfd, evs, size, -1);
        cout<<"num = "<<num<<endl;
        for(int i=0; i<num; ++i)
        {
            int fd = evs[i].data.fd; 
            if(fd == lfd)
            {
                sockaddr_in addr;
                TcpSocket* tcp = s->acceptConn(&addr);
                //tcp->Close();
                int cfd = tcp->getSocket();
                ev.events = EPOLLIN;
                ev.data.fd = cfd;
                int ec = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
                delete tcp;
            }
            else
            {
                printf("接收数据...\n");
                TcpSocket tcp(fd);
                string msg = tcp.recvMsg();
                if (!msg.empty())
                {
                    cout << msg << endl << endl;
                }
                else
                {
                    tcp.Close();
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    cout<<"客户端关闭连接"<<endl;
                    break;
                }
                
                int ret = tcp.sendMsg("ok");
                
                if(ret == -1)
                {
                    tcp.Close();
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    perror("send error");
                    break;
                }
                cout<<"发送成功"<<endl;
                //tcp.Close();
            }
        }
    }
    close(lfd);

    // ThreadPool<SockInfo>* pool =  new ThreadPool<SockInfo>(3,100);

    // PoolInfo* info = new PoolInfo;
    // info->s = s;
    // info->pool = pool;
    // Task<SockInfo> task(acceptconn, info);
    // pool->threadPoolAdd(task); // 接受连接也是任务，会放到任务队列中
    
    // pthread_exit(NULL);

    return 0;

    // while (1)
    // {

    //     SockInfo* info = new SockInfo;
    //     info->s = &s;
    //     pthread_t aid;
    //     //TcpSocket* tcp = s.acceptConn(&info->addr);
    //     //pthread_create(&aid, NULL, accepting, info);
    //     //pthread_detach(aid);
    //     accepting(info);
    //     if (info->tcp == nullptr)
    //     {
    //         cout << "重试...." << endl;
    //         continue;
    //     }
        // 创建子线程
        // pthread_t tid;
        // //info->s = &s;
        // //info->tcp = tcp;
        // Task<SockInfo> task(working, info);
        // //pthread_create(&tid, NULL, working, info);
        // pthread_detach(tid);
    //}
}