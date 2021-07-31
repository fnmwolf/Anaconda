import tkinter as tk
from tkinter import filedialog, Text
from tkinter import *
import os
import time
from threading import *

root = tk.Tk()
root.title("Anaconda Mode-4")
root.geometry("600x400")
root.resizable(0,0)

cmd = "decompile.bat"
app = 'null'
folder = 'null'

def addApp():
	for widget in frame1.winfo_children():
		widget.destroy()

	global app

	filename = filedialog.askopenfilename(initialdir="/", title="Select File", filetypes=(("Executables","*.exe"), ("All Files (Not recommended)", "*.*")))

	app = filename
	print(app)
	label = tk.Label(frame1, text=app, fg="orange", bg="#222222", pady=6)
	label.pack()

def addDir():
	for widget in frame2.winfo_children():
		widget.destroy()

	global folder

	filename = filedialog.askdirectory()

	folder = filename
	print(filename)
	label = tk.Label(frame2, text=folder, fg="orange", bg="#222222", pady=6)
	label.pack()

def decompile():
	DecompileButton.config(text="Working...")
	root.update()
	t1=Thread(target=decompiler)
	t1.start()

def decompiler():
	openGame.config(state = DISABLED)
	openGame2.config(state = DISABLED)
	DecompileButton.config(state = DISABLED)
	os.system(cmd+' "'+app+'" "'+folder+'"')
	DecompileButton.config(text="Completed!")
	root.update()
	time.sleep(2)
	openGame.config(state = NORMAL)
	openGame2.config(state = NORMAL)
	DecompileButton.config(state = NORMAL)
	time.sleep(3)
	DecompileButton.config(text="Decompile")

		
def runApps():
	for app in apps:
		os.startfile(app)

canvas = tk.Canvas(root, height = 400, width = 600, bg = "#121212")
canvas.pack()

Title = tk.Label(root, text="Anaconda-Mode 4", width=0, height=0, font=("Arial", 30), bg="#121212", fg="white")
Title.place(relx=.5, rely=0.09, anchor="center")

frame1 = tk.Frame(root, bg="#222222")
frame1.place(relwidth=0.4, relheight=0.07, relx=0.2, rely=0.3)

openGame = tk.Button(root, text="Open Game", padx=10, pady=5, fg="orange", bg="#222222", command=addApp) 
openGame.place(x=370, y=117.75)
openGameLabel = tk.Label(root, text="Select your game", width=0, height=0, font=("Arial", 20), bg="#121212", fg="orange")
openGameLabel.place(relx=.5, rely=0.2, anchor="center")

frame2 = tk.Frame(root, bg="#222222")
frame2.place(relwidth=0.4, relheight=0.07, relx=0.2, rely=0.59)

openGame2 = tk.Button(root, text="Select Directory", padx=10, pady=5, fg="orange", bg="#222222", command=addDir) 
openGame2.place(x=370, y=232.5)
openGame2Label = tk.Label(root, text="Select your output directory", width=0, height=0, font=("Arial", 20), bg="#121212", fg="orange")
openGame2Label.place(relx=.5, rely=0.5, anchor="center")

DecompileButton = tk.Button(root, text="Decompile", padx=20, pady=10, font=("Arial",15), fg="orange", bg="#222222", command=decompile) 
DecompileButton.place(relx=.5, rely=0.9, anchor="center")

root.mainloop()
