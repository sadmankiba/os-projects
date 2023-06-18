from collections import deque
import shutil, os
from mfs import *

import toolspath
from testing.build import *
from testing.test import Test, Failure

class WriteAction:
   def __init__(self, inum, block, data):
      self.inum = inum
      self.block = block
      self.data = data
   def do(self, mfs):
      mfs.write(self.inum, gen_block(self.data), self.block)
      return list()

class File:
   def __init__(self, type, name):
      self.name = name
      self.type = type
      self.isroot = False
   def fullpath(self):
      if self.isroot:
         return ""
      else:
         return self.parent.fullpath() + "/" + self.name
   def do(self, mfs):
      if not self.isroot:
         mfs.creat(self.parent.inum, self.type, self.name)
         self.inum = mfs.lookup(self.parent.inum, self.name)
      return list()

class RegularFile(File):
   def __init__(self, name, blocks):
      File.__init__(self, MFS_REGULAR_FILE, name)
      self.blocks = blocks
      while len(self.blocks) > 0 and self.blocks[-1] is None:
         self.blocks.pop()
   def size(self):
      return len(self.blocks) * MFS_BLOCK_SIZE
   def do(self, mfs):
      File.do(self, mfs)
      actions = list()
      for i in range(len(self.blocks)):
         if self.blocks[i] is not None:
            actions.append(WriteAction(self.inum, i, self.blocks[i]))
      return actions
   def check(self, mfs):
      mfs.log("checking " + self.fullpath() + "\n")
      for i in range(len(self.blocks)):
         if self.blocks[i] is not None:
            mfs.read_and_check(self.inum, i, gen_block(self.blocks[i]))
      st = mfs.stat(self.inum)
      if st.size != self.size():
         raise Failure("Incorrect size for file " + self.fullpath())

   def __str__(self):
      return self.fullpath() + " " + str(self.blocks)

class Dir(File):
   @staticmethod
   def root():
      r = Dir("")
      r.isroot = True
      r.inum = ROOT
      return r
   def __init__(self, name):
      File.__init__(self, MFS_DIRECTORY, name)
      self.name = name
      self.children = dict()
   def add_child(self, child):
      if len(self.children) >= MAX_FILES_PER_DIR:
         return None
      self.children[child.name] = child
      child.parent = self
      return child
   def do(self, mfs):
      File.do(self, mfs)
      return list(self.children.values())
   def check(self, mfs):
      for child in list(self.children.values()):
         child_inum = mfs.lookup(self.inum, child.name)
         if child_inum != child.inum:
            raise Failure("Incorrect inum for " + child.fullpath())
         child.check(mfs)
   def __str__(self):
      return "\n".join([self.fullpath()] +
            [str(child) for child in list(self.children.values())])


class PersistTest(MfsTest):
   name = "persist"
   description = "restart server after creating a file"
   timeout = 30
   def run(self):
      image = self.create_image()
      self.loadlib()
      self.start_server(image)
      self.mfs_init("localhost", self.port)

      self.creat(ROOT, MFS_REGULAR_FILE, "test")
      inum = self.lookup(ROOT, "test")
      self.write(inum, gen_block(1), 0, MFS_BLOCK_SIZE)

      if self.lookup(ROOT, "test") != inum:
         raise Failure("Wrong inum")
      self.read_and_check(inum, 0, gen_block(1), MFS_BLOCK_SIZE)

      self.shutdown()
      self.server.wait()

      self.mfs_init("localhost", self.port)
      self.start_server(image, port=self.port)
      if self.lookup(ROOT, "test") != inum:
         raise Failure("Wrong inum")
      self.read_and_check(inum, 0, gen_block(1), MFS_BLOCK_SIZE)

      self.shutdown()
      self.server.wait()
      self.done()


class BigDirTest(MfsTest):
   name = "bigdir"
   description = "create a directory with 126 files"
   timeout = 180

   def run(self):
      image = self.create_image_max(32, 128)
      self.loadlib()
      self.start_server(image)
      self.mfs_init("localhost", self.port)

      self.creat(ROOT, MFS_DIRECTORY, "testdir")
      inum = self.lookup(ROOT, "testdir")

      for i in range(MAX_FILES_PER_DIR):
         self.creat(inum, MFS_REGULAR_FILE, str(i))

      for i in range(MAX_FILES_PER_DIR):
         self.lookup(inum, str(i))

      self.shutdown()
      self.server.wait()
      self.done()

class DeepTest(MfsTest):
   name = "deep"
   description = "create many deeply nested directories"
   timeout = 60

   depth = 4 

   def run(self):
      image = self.create_image_max(32,128)
      self.loadlib()
      self.start_server(image)
      self.mfs_init("localhost", self.port)

      self.creat(ROOT, MFS_DIRECTORY, "testdir")
      
      inum = self.lookup(ROOT, "testdir")

      for i in range(self.depth):
         self.creat(inum, MFS_DIRECTORY, str(i))
         inum = self.lookup(inum, str(i))

      inum = self.lookup(ROOT, "testdir")
      for i in range(self.depth):
         inum = self.lookup(inum, str(i))

      self.shutdown()
      self.server.wait()
      self.done()

test_list = [PersistTest, BigDirTest, DeepTest]
# test_list = []
