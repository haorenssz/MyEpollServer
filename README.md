# Epoll+线程池实现tcp服务器

## socket模块

tcpSocket.h和tcpSocket.c封装socket基本操作

## Epoll

采用ET触发模式

## 线程池

采用https://github.com/progschj/ThreadPool的线程池

本线程池使用C++11编写，接口灵活
