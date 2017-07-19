#include "Connection.hpp"

namespace socketx{

    Connection::Connection(EventLoop *loop, int fd):
        loop_(loop),
        socketfd(fd),
        event_(new Event(loop)){
        /*Set callback functions*/
        event_->setReadFunc(std::bind(&Connection::handleRead, this));
        event_->setWriteFunc(std::bind(&Connection::handleWrite, this));
        event_->setErrorFunc(std::bind(&Connection::handleError, this));
        /*Update socketfd*/
        event_->setFD(socketfd);
        rio_readinitb(socketfd);
    }

    void Connection::handleRead(){
        handleReadEvents(this);
    }

    void Connection::handleWrite(){
        handleWriteEvents(this);
    }

    /*Connect the file descriptor to rio struct*/
    void communication::rio_readinitb(int fd){
        rio.rio_fd = fd;  
        rio.rio_cnt = 0;  
        rio.rio_bufptr = rio.rio_buf;
    }

    /* 
    * rio_read - This is a wrapper for the Unix read() function that
    *    transfers min(n, rio_cnt) bytes from an internal buffer to a user
    *    buffer, where n is the number of bytes requested by the user and
    *    rio_cnt is the number of unread bytes in the internal buffer. On
    *    entry, rio_read() refills the internal buffer via a call to
    *    read() if the internal buffer is empty.
    */
    ssize_t Connection::rio_read(char *usrbuf, size_t n){
        int cnt;

        while (rio.rio_cnt <= 0) {  /* Refill if buf is empty */
            rio.rio_cnt = read(rio.rio_fd, rio.rio_buf, 
                    sizeof(rio.rio_buf));
            if (rio.rio_cnt < 0) {
                if (errno != EINTR) /* Interrupted by sig handler return */
                return -1;
            }
            else if (rio.rio_cnt == 0)  /* EOF */
                return 0;
            else 
                rio.rio_bufptr = rio.rio_buf; /* Reset buffer ptr */
        }

        /* Copy min(n, rp->rio_cnt) bytes from internal buf to user buf */
        cnt = n;          
        if (rio.rio_cnt < n)   
            cnt = rio.rio_cnt;
        memcpy(usrbuf, rio.rio_bufptr, cnt);
        rio.rio_bufptr += cnt;
        rio.rio_cnt -= cnt;
        return cnt;
    }

    /*Send *n* bytes of buffer to the host it connected*/
    ssize_t Connection::send(const void *buffer, size_t n){
        size_t nleft = n;
        ssize_t nwritten;
        char *bufp = (char *)buffer;

        while (nleft > 0) {
            if ((nwritten = write(socketfd, bufp, nleft)) <= 0) {
                if (errno == EINTR)  /* Interrupted by sig handler return */
                    nwritten = 0;    /* and call write() again */
                else
                    return -1;       /* errno set by write() */
            }
            nleft -= nwritten;
            bufp += nwritten;
        }
        return n;
    }

    /*Receive bytes from the host it connected.
    * Save bytes to usrbuf with length n.
    */
    ssize_t Connection::recvFromBuffer(void *usrbuf, size_t n){
        size_t nleft = n;
        ssize_t nread;
        char *bufp = (char *)usrbuf;
        
        while (nleft > 0) {
            if ((nread = rio_read(bufp, nleft)) < 0) 
                    return -1;          /* errno set by read() */ 
            else if (nread == 0)
                break;              /* EOF */
            nleft -= nread;
            bufp += nread;
        }
        return (n - nleft);         /* return >= 0 */
    }

    /*Override function of recv.
    * This function needs a fd parameter,
    * It does not use the internal buffer.
    */
    ssize_t Connection::recv(void *usrbuf, size_t n){
        size_t nleft = n;
        ssize_t nread;
        char *bufp = (char *)usrbuf;

        while (nleft > 0) {
            if ((nread = read(socketfd, bufp, nleft)) < 0) {
                if (errno == EINTR) /* Interrupted by sig handler return */
                    nread = 0;      /* and call read() again */
                else
                    return -1;      /* errno set by read() */ 
            } 
            else if (nread == 0)
                break;              /* EOF */
            nleft -= nread;
            bufp += nread;
        }
        return (n - nleft);         /* Return >= 0 */
    }

    ssize_t Connection::readline(void *usrbuf, size_t maxlen){
        int n, rc;
        char c, *bufp = (char *)usrbuf;

        for (n = 1; n <= maxlen; n++) { 
            if ((rc = rio_read(&c, 1)) == 1) {
                *bufp++ = c;
                if (c == '\n') {
                    n++;
                    break;
                }
            } else if (rc == 0) {
                if (n == 1)
                    return 0; /* EOF, no data read */
                else
                    break;    /* EOF, some data was read */
            } else
            return -1;	  /* Error */
        }
        *bufp = 0;
        return n-1;
    }
    

    /*Send and receive messages*/
    ssize_t Connection::sendmsg(const message &msg){
        size_t n = msg.get_size();
        char * buffer = msg.get_data();
        /*Send the size of the message first*/
        if(send(socketfd,&n,sizeof(n))>0){
            /*Send data*/
            if(send(socketfd,buffer,n)>0)
                return 1;
        }
        return -1;
    }

    Message Connection::recvmsgFromBuffer(){
        size_t n = 0;
        /*Receive the size of the message*/
        if(recvFromBuffer(&n,sizeof(size_t))>0){
            /*Recieve the message*/
            char * data = new char[n];
            recvFromBuffer(data,n);
            return message(data,n);
        }
        else return message(nullptr,0);
    }

     Message Connection::recvmsg(){
        size_t n = 0;
        /*Receive the size of the message*/
        if(recv(socketfd, &n,sizeof(size_t))>0){
            /*Recieve the message*/
            char * data = new char[n];
            recv(socketfd, data,n);
            return message(data,n);
        }
        else return message(nullptr,0);
    }
}