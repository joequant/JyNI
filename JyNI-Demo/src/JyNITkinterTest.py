import sys
sys.path.insert(0, '/usr/lib/python2.7/lib-dynload')
#sys.path.insert(0, '../../../Tkinter/Tkinter-C/Debug')
#import _tkinter
sys.path.insert(0, '/usr/lib/python2.7/lib-tk')
#sys.path.insert(0, '../../../Tkinter/Tkinter-Python')

from Tkinter import *

def printTest():
	print "test"
	
def printTest2():
	print "test2"
	
def printTimeStamp():
	from java.lang import System
	print "System.currentTimeMillis: "+str(System.currentTimeMillis())

root = Tk()
Label(root, text = "Welcome to JyNI Tkinter-Demo!").pack()
Button(root, text="print \"test\"", command=printTest).pack()
Button(root, text="print timestamp", command=printTimeStamp).pack()
Button(root, text="print \"test2\"", command=printTest2).pack()
# #Button(root, text="Quit", command=lambda root=root:quit(root)).pack()
Button(root, text="Quit", command=root.destroy).pack()

root.mainloop()
print "===="
print "exit"
print "===="