from mfs import *

class ShutdownTest(MfsTest):
   name = "shutdown"
   description = "init server and client then call shutdown"
   timeout = 10

   def run(self):
      image = self.create_image()
      self.loadlib()
      self.start_server(image)
      self.mfs_init("localhost", self.port)
      self.shutdown()
      self.server.wait()
      self.done()

test_list = [ShutdownTest]



