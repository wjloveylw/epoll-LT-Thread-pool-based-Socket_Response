#include "response.h"

int main()
{
    // 1. 创建通信的套接字
    TcpSocket tcp;

    // 2. 连接服务器IP port
    int ret = tcp.connectToHost("192.168.200.100", 8080);
    if (ret == -1)
    {
        return -1;
    }

    // // 3. 通信
    // int fd1 = open("english.txt", O_RDONLY);
    // int length = 0;
    // char tmp[100];
    // memset(tmp, 0, sizeof(tmp));
    // while ((length = read(fd1, tmp, sizeof(tmp))) > 0)
    // {
    //     // 发送数据
    //     tcp.sendMsg(string(tmp, length));

    //     cout << "send Msg: " << endl;
    //     cout << tmp << endl << endl << endl;
    //     memset(tmp, 0, sizeof(tmp));

    //     // 接收数据
    //     usleep(300);
    // }

    // sleep(10);

    std::string input;
	char* buf;
	for(;;)
	{
		std::cout<<"输入字符串,退出请输入quit或q："<<std::endl;
		std::cin>>input;
		if(input == "quit" || input == "q") 
        {
            cout<<"断开连接"<<endl;
            break;
        }

		tcp.sendMsg(input.c_str());
		for(;;)
		{
			string r = tcp.recvMsg();
			if(r.empty()!=1)
			{
				std::cout<<"recv: "<<r<<std::endl;
			}
            break;
		}
	} 
    //tcp.Close();
    return 0;
}

