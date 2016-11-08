package main

import (
	"fmt"
	"golang.org/x/net/websocket"
	
	"log"
	"net/http"
	"net"
	"io"
	"os"
	"time"
	"bufio"
	"encoding/binary"
	"bytes"
	"io/ioutil"
)

var g_websocket *websocket.Conn ;


func wsh264file(ws *websocket.Conn) {
	fmt.Printf("new socket\n")
	
	for i := 1; i < 40; i++ {
		strpath := fmt.Sprintf("D:/file/%d.mp4", i)
		fmt.Println(strpath)
		data, err := ioutil.ReadFile(strpath)
		if err != nil {
		fmt.Println("read error")
		os.Exit(1)
		}	    
	    // write to the websocket
	 
	  err = websocket.Message.Send(ws, data)
		if err != nil {
				log.Println(err)
				break;
	  }
	  time.Sleep(1000 * time.Millisecond)
	}    
	
}
func wsH264(ws *websocket.Conn) {
	fmt.Printf("new socket\n")

	fi, err := os.Open("./test.h264")
	if err != nil {
		log.Fatal(err)
	}
	defer fi.Close()
  
	msg := make([]byte, 1024*512)	
	lenBytes := make([]byte, 4)
	for {
		time.Sleep(40 * time.Millisecond)
		
		lenNum, err := fi.Read(lenBytes)
		if (err != nil && err != io.EOF) || lenNum != 4 {
			log.Println(err)
			time.Sleep(1 * time.Second)
			break
		}
		
		b_buf := bytes.NewBuffer(lenBytes)
    var lenreal int32
    binary.Read(b_buf, binary.LittleEndian, &lenreal)
    
		log.Println(lenreal)
		
		n, err := fi.Read(msg[0:lenreal])
		if (err != nil && err != io.EOF) || n != int(lenreal) {
			log.Println(err)
			time.Sleep(1 * time.Second)
			break
		}
		
		err = websocket.Message.Send(ws, msg[:n])
		if err != nil {
			log.Println(err)
			break
		}
	}

	log.Println("send over socket\n")
}

func wsflv(ws *websocket.Conn) {
	fmt.Printf("new socket\n")

	fi, err := os.Open("./test.flv")
	if err != nil {
		log.Fatal(err)
	}
	defer fi.Close()
  
	msg := make([]byte, 1024*5)	
	
	for {
		time.Sleep(4 * time.Millisecond)
		
		/*lenNum, err := fi.Read(lenBytes)
		if (err != nil && err != io.EOF) || lenNum != 4 {
			log.Println(err)
			time.Sleep(1 * time.Second)
			break
		}*/
		
		
    var lenreal int32
    lenreal = 1024*5;
    
		log.Println(lenreal)
		
		n, err := fi.Read(msg[0:lenreal])
		if (err != nil && err != io.EOF) || n != int(lenreal) {
			log.Println(err)
			time.Sleep(1 * time.Second)
			break
		}
		
		err = websocket.Message.Send(ws, msg[:n])
		if err != nil {
			log.Println(err)
			break
		}
	}

	log.Println("send over socket\n")
}

func wsMpeg1(ws *websocket.Conn) {
	fmt.Printf("new socket\n")

	buf := make([]byte, 10)
	buf[0] = 'j'
	buf[1] = 's'
	buf[2] = 'm'
	buf[3] = 'p'
	buf[4] = 0x01
	buf[5] = 0x40
	buf[6] = 0x0
	buf[7] = 0xf0
	websocket.Message.Send(ws, buf[:8])

	fi, err := os.Open("./test.mpeg")
	if err != nil {
		log.Fatal(err)
	}
	defer fi.Close()

	msg := make([]byte, 1024*1)
	for {
		time.Sleep(40 * time.Millisecond)
		n, err := fi.Read(msg)
		if err != nil && err != io.EOF {
			log.Fatal(err)
		}
		if 0 == n {
			time.Sleep(1 * time.Second)
			break
		}
		err = websocket.Message.Send(ws, msg[:n])
		if err != nil {
		   log.Println(err)
		   break
		}
	}
	fmt.Printf("send over socket\n")	
}








/////////////////////////////////////////////////////////

type connection struct {
    ws *websocket.Conn
    send chan []byte
}

func (c *connection) reader() {
   r := bufio.NewReader(c.ws)
   for {
    	_, err := r.ReadBytes('\n')
    	if err != nil {
        	break
    	}
    }
    
    c.ws.Close()
}

func (c *connection) writer() {
    for message := range c.send {
        err := websocket.Message.Send(c.ws, message)
        if err != nil {
            break
        }
    }
    c.ws.Close()
}


func wsdashlive(ws *websocket.Conn) {

    c := &connection{send: make(chan []byte, 3), ws: ws}
    h.register <- c
    defer func() { h.unregister <- c }()
    go c.writer()
    c.reader()
}

type hub struct {
    connections map[*connection]bool

    broadcast chan []byte

    register chan *connection

    unregister chan *connection
}

var h = hub{
    broadcast:   make(chan []byte),
    register:    make(chan *connection),
    unregister:  make(chan *connection),
    connections: make(map[*connection]bool),
}

func (h *hub) run() {
    for {
        select {
        case c := <-h.register:
            h.connections[c] = true
        case c := <-h.unregister:
            if _, ok := h.connections[c]; ok {
                delete(h.connections, c)
                close(c.send)
            }
        case m := <-h.broadcast:
            for c := range h.connections {
                select {
                case c.send <- m:
                default:
                    delete(h.connections, c)
                    close(c.send)
                }
            }
        }
    }
}





func handleConn(c net.Conn) {
    defer c.Close()
    log.Println("new tcp conn")
    
    bufFrame := make([]byte, 3*1024*1024);
	  lenbuf := make([]byte, 4);
	    
    for {
	    
	    
	    _, err := io.ReadFull(c, lenbuf);
	    if err != nil {
	    	log.Println(err)
	    	break
	    }
	    
	    b_buf := bytes.NewBuffer(lenbuf)
	    var lenreal int32
	    binary.Read(b_buf, binary.LittleEndian, &lenreal)
	    
	    fmt.Printf("buf len is %d\n", lenreal)
	    
	    _, err = io.ReadFull(c, bufFrame[:lenreal]);
	    if err != nil {
	    	log.Println(err)
	    	break;
	    }
	    
	    h.broadcast <- bufFrame[:lenreal]
	    
	    // write to the websocket
	    /*if nil != g_websocket {
	    	err := websocket.Message.Send(g_websocket, bufFrame[:lenreal])
				if err != nil {
				   log.Println(err)
				   //break
				}
	    }*/
	    
    }
    
}

func localTcp() {
	 l, err := net.Listen("tcp", ":8888")
    if err != nil {
        fmt.Println("listen error:", err)
        return
    }

    for {
        c, err := l.Accept()
        if err != nil {
            fmt.Println("accept error:", err)
            break
        }
        // start a new goroutine to handle
        // the new connection.
        go handleConn(c)
    }
	
}



///////////////////////////////////////////////////////





func main() {

	go localTcp()
	go h.run()
	
	http.Handle("/wsh264", websocket.Handler(wsH264))
	http.Handle("/wsdashlive", websocket.Handler(wsdashlive))
	http.Handle("/wsmpeg", websocket.Handler(wsMpeg1))
	http.Handle("/wsflv", websocket.Handler(wsflv))
	http.Handle("/wsh264file", websocket.Handler(wsh264file))
	
	http.Handle("/", http.FileServer(http.Dir("./public")))

	err := http.ListenAndServe(":8080", nil)

	if err != nil {
		panic("ListenAndServe: " + err.Error())
	}
}
