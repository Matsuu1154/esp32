import ctypes
import ctypes.util

# glibc をロード
libc = ctypes.CDLL(ctypes.util.find_library("c"))

# mq_open の定義
mq_open = libc.mq_open
mq_open.argtypes = [ctypes.c_char_p, ctypes.c_int]
mq_open.restype = ctypes.c_int

# mq_receive の定義
mq_receive = libc.mq_receive
mq_receive.argtypes = [ctypes.c_int, ctypes.c_char_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_uint)]
mq_receive.restype = ctypes.c_int

# mq_close
mq_close = libc.mq_close

# キューを開く（C と同じ名前）
mq_name = b"/my_queue"
O_RDONLY = 0x0000

mqdes = mq_open(mq_name, O_RDONLY)
if mqdes == -1:
    raise OSError("mq_open failed")

# メッセージ受信用バッファ
MSG_SIZE = 1024
buf = ctypes.create_string_buffer(MSG_SIZE)
prio = ctypes.c_uint()

while(1):
    key = input("キー入力待ち中")

    # 受信（ブロッキング）
    ret = mq_receive(mqdes, buf, MSG_SIZE, ctypes.byref(prio))
    if ret >= 0:
        # msg_str = buf.value.decode().strip()   # "123" のような文字列になる
        # msg_int = int(msg_str)
        print("received:", buf.value.decode(), "prio:", prio.value)
    else:
        raise OSError("mq_receive failed")


mq_close(mqdes)
