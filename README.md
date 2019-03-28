# ClientAndServerDemo

>```server.cpp```参考自csr_2015 

[C实现简单web服务器-1](https://blog.csdn.net/u012291157/article/details/46391189)


>```server2.c```已完成

---
# 使用说明
+ 命令行打开```server.cpp```，无需输入任何参数
+ 命令行打开```server2.c```，参数为端口号输入8080
    >形如```\server2.exe 8080```
+ 命令行打开```client.cpp```,不需要参数
+ 客户端向服务器发送请求
	>形如```GET /time.html HTTP/1.0```

# 可以进行的操作
+ ```GET /time.html HTTP/1.0```访问time.html页面
+ ```GET / HTTP/1.0```访问homepage页面
+ 若发送格式有问题，返回400页面
+ 若请求文件不存在，返回404页面
+ 若要进行类似```GET /1.html HTTP/1.0```的操作，请将1.html置于exe文件的同级目录。

# 其他
在浏览器中访问服务器时，仍然存在BUG，欢迎大家修改
