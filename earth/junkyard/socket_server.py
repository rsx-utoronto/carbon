# Echo server program
import socket

HOST = ''                 # Symbolic name meaning all available interfaces
PORT = 50007              # Arbitrary non-privileged port
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
s.listen(1)
conn, addr = s.accept()
print 'Connected by', addr
while 1:
    try:
	    print conn.recv(1024)
    except Exception as e:
       pass
    # if not data: pass
    try:
        conn.sendall("Test")
    except Exception as e:
	     pass
	
# conn.close()
