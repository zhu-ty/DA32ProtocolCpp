1、json文件夹中的.h文件均为系统自带，直接调用即可
2、server和client收发
3、md5.cpp做md5校验和
4、MyJson.cpp把输入信息传成Jason包
5、DA32Protocol.h所有需要的预编译文件、需要维护的Client列表和Server
6、MyTson.h定义了信息层类MyJson
7、message.cpp封装Json，加头尾
8、usinginmain.h中写了主函数中的各种功能
9、main.cpp主函数
**********************************************************************
usinginmain.h（供main调用实现主函数功能）
bool findClient(Client *tofind)返回是否可以找到一个Client（1为找到）
void mianThread()是主线程，输入、验证和创建Client
void quitFunction()实现安全退出
Client* checkClientList(SOCKADDR_IN add)检查客户端链接表，返回当前客户端
void deleteClient(Client* c)删除客户端
void initServer()各种初始化。。。
**********************************************************************
MyJson.cpp
tm convert_string_to_time_t(const std::string & time_string)将固定格式的字符串解析成时间结构tm
Json类函数的定义
bool MyJson::getJson(std::string charflow)从字符流中获得一个Json返回1为成功
bool MyJson::getJson(istream& charflow)从文件中读入一个Json返回1成功
void MyJson::showJson_in_console()输出Json
string MyJson::PackJson(std::string input)打包
**********************************************************************
Message.cpp
GBKToUTF8和UTF8ToGBK:GBK编码和UTF8编码转换
MyJson Message::getContent(string dataflow)解包，获得Json
string Message::getWrap(string tosend)封装，返回UTF8
**********************************************************************