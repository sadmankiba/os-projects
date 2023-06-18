from mfs import *
import subprocess
import toolspath
from testing.test import Failure

class WriteTest(MfsTest):
   name = "write"
   description = "write then read one block"
   timeout = 10

   def run(self):
      image = self.create_image()
      self.loadlib()
      self.start_server(image)

      self.mfs_init("localhost", self.port)
      self.creat(0, MFS_REGULAR_FILE, "test")
      inum = self.lookup(0, "test")

      buf1 = gen_block(1)
      self.write(inum, buf1, 0, MFS_BLOCK_SIZE)

      buf2 = BlockBuffer()
      self.read(inum, buf2, 0, MFS_BLOCK_SIZE)

      print(buf1[:40], buf1[-40:])
      print(buf2[:40], buf2[-40:])
      if not bufs_equal(buf1, buf2):
         raise Failure("Corrupt data returned by read")

      self.shutdown()

      self.server.wait()
      self.done()

class StatTest(MfsTest):
   name = "stat"
   description = "stat a regular file"
   timeout = 10
   
   def run(self):
      
      image = self.create_image()
      self.loadlib()
      self.start_server(image)

      self.mfs_init("localhost", self.port)
      self.creat(ROOT, MFS_REGULAR_FILE, "test")
      inum = self.lookup(ROOT, "test")

      st = self.stat(ROOT)
      if st.type != MFS_DIRECTORY:
         raise Failure("Stat gave wrong type")

      st = self.stat(inum)
      if st.size != 0:
         raise Failure("Stat gave wrong size")
      if st.type != MFS_REGULAR_FILE:
         raise Failure("Stat gave wrong type")

      buf1 = gen_block(1)
      self.write(inum, buf1, 0, MFS_BLOCK_SIZE)

      st = self.stat(inum)
      if st.size != MFS_BLOCK_SIZE:
         raise Failure("Stat gave wrong size")
      if st.type != MFS_REGULAR_FILE:
         raise Failure("Stat gave wrong type")

      self.shutdown()
      self.server.wait()
      self.done()


class OverwriteTest(MfsTest):
   name = "overwrite"
   description = "overwrite a block"
   timeout = 10

   def run(self):
      image = self.create_image()
      self.loadlib()
      self.start_server(image)

      self.mfs_init("localhost", self.port)
      self.creat(0, MFS_REGULAR_FILE, "test")
      inum = self.lookup(0, "test")

      buf1 = gen_block(1)
      self.write(inum, buf1, 0, MFS_BLOCK_SIZE)
      self.read_and_check(inum, 0, buf1, MFS_BLOCK_SIZE)

      buf2 = gen_block(2)
      self.write(inum, buf2, 0, MFS_BLOCK_SIZE)
      self.read_and_check(inum, 0, buf2, MFS_BLOCK_SIZE)

      self.shutdown()

      self.server.wait()
      self.done()

class MaxFileTest(MfsTest):
   name = "maxfile"
   description = "write largest possible file"
   timeout = 10

   def run(self):
      image = self.create_image_max(32, 128)
      self.loadlib()
      self.start_server(image)

      self.mfs_init("localhost", self.port)
      self.creat(0, MFS_REGULAR_FILE, "test")
      inum = self.lookup(0, "test")

      buf = [gen_block(i) for i in range(MAX_FILE_BLOCKS)]
      # buf = gen_block(1)

      for i in range(MAX_FILE_BLOCKS):
         self.write(inum, buf[i], i * MFS_BLOCK_SIZE, MFS_BLOCK_SIZE)

      for i in range(MAX_FILE_BLOCKS):
         self.read_and_check(inum, i * MFS_BLOCK_SIZE, buf[i], MFS_BLOCK_SIZE)

      self.shutdown()

      self.server.wait()
      self.done()


class MaxFile2Test(MfsTest):
   name = "maxfile2"
   description = "write more blocks than possible"
   timeout = 10

   def run(self):
      image = self.create_image()
      self.loadlib()
      self.start_server(image)
      self.mfs_init("localhost", self.port)
      self.creat(0, MFS_REGULAR_FILE, "test")
      inum = self.lookup(0, "test")

      buf = [gen_block(i) for i in range(MAX_FILE_BLOCKS + 1)]
      print len(buf)
      for i in range(MAX_FILE_BLOCKS):
         # print "write = " + str(i)
         self.write(inum, buf[i], i*MFS_BLOCK_SIZE, MFS_BLOCK_SIZE)
      print "write more than need"
      r = self.libmfs.MFS_Write(inum, byref(buf[MAX_FILE_BLOCKS]), MAX_FILE_BLOCKS * MFS_BLOCK_SIZE, MFS_BLOCK_SIZE)
      if r != -1:
         raise Failure("MFS_Write should fail on inalid block number")

      for i in range(MAX_FILE_BLOCKS):
         self.read_and_check(inum, i*MFS_BLOCK_SIZE, buf[i], MFS_BLOCK_SIZE)
      r = self.libmfs.MFS_Read(inum, byref(buf[MAX_FILE_BLOCKS]), MAX_FILE_BLOCKS * MFS_BLOCK_SIZE, MFS_BLOCK_SIZE)
      if r != -1:
         raise Failure("MFS_Read should fail on inalid block number")

      self.shutdown()

      self.server.wait()
      self.done()

test_list = [WriteTest, StatTest, OverwriteTest, MaxFileTest, MaxFile2Test]

