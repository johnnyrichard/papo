# Papo IRC Server
# Copyright (C) 2021 Johnny Richard
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

import ctypes
import socket
import threading
import time
import unittest


libpapo = ctypes.CDLL('./libpapo.so')
unittest.util._MAX_LENGTH=2000


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
            s.sendall(b'USER username 0 * :User Name\r\n')
            data = s.recv(1024)
            self.assertIn(b'001 username :Welcome!\r\n', data)


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
