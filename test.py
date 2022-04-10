import ctypes
import socket
import threading
import time
import unittest

libpapo = ctypes.CDLL('./libpapo.so')

class PapoServerTest(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        server = papo_server();
        thread = threading.Thread(target=cls.run_papo_server, args=(server,))
        thread.start()
        time.sleep(2)

    @classmethod
    def tearDownClass(cls):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect(('localhost', 6667))
            s.sendall(b'exit')

    @classmethod
    def run_papo_server(cls, server):
        libpapo.server_init(ctypes.byref(server), 6667)
        libpapo.server_run(ctypes.byref(server))

    def test_server_run(self):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect(('localhost', 6667))
            s.sendall(b'A msg from client')
            data = s.recv(1024)
            self.assertIn(b':A msg from client', data)

class epoll_data(ctypes.Union):
    _fields_ = [('ptr', ctypes.c_void_p),
                ('fd', ctypes.c_int),
                ('u32', ctypes.c_uint),
                ('u64', ctypes.c_ulong)]

class epoll_event(ctypes.Structure):
    _fields_ = [('events', ctypes.c_uint),
                ('epoll_data_t', epoll_data)]


class papo_server(ctypes.Structure):
    _MAXEVENTS=64
    _fields_ = [('fd', ctypes.c_int),
                ('epoll_fd', ctypes.c_int),
                ('events', epoll_event * _MAXEVENTS),
                ('connected_clients', ctypes.c_int * _MAXEVENTS),
                ('running', ctypes.c_bool)]


if __name__ == '__main__':
    unittest.main()
