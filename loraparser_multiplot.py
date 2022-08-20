#!/usr/bin/env python

from pickle import TRUE
from threading import Thread
import serial
import time
import collections
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import struct
import copy
import tkinter as Tk
from tkinter.ttk import Frame
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2Tk
import PySimpleGUI as sg 

class serialPlot:
    def __init__(self, serialPort='/dev/ttyUSB0', serialBaud=38400, plotLength=100, dataNumBytes=2, numPlots=1):
        self.port = serialPort
        self.baud = serialBaud
        self.plotMaxLength = plotLength
        self.dataNumBytes = dataNumBytes
        self.numPlots = numPlots
        self.rawData = bytearray(numPlots * dataNumBytes)
        self.dataType = None
        if dataNumBytes == 2:
            self.dataType = 'h'     # 2 byte integer
        elif dataNumBytes == 4:
            self.dataType = 'f'     # 4 byte float
        self.data = []
        self.privateData = None     # for storing a copy of the data so all plots are synchronized
        for i in range(numPlots):   # give an array for each type of data and store them in a list
            self.data.append(collections.deque([0] * plotLength, maxlen=plotLength))
        self.isRun = True
        self.isReceiving = False
        self.thread = None
        self.plotTimer = 0
        self.previousTimer = 0

        print('Trying to connect to: ' + str(serialPort) + ' at ' + str(serialBaud) + ' BAUD.')
        try:
            self.serialConnection = serial.Serial(serialPort, serialBaud, timeout=4)
            print('Connected to ' + str(serialPort) + ' at ' + str(serialBaud) + ' BAUD.')
        except:
            print("Failed to connect with " + str(serialPort) + ' at ' + str(serialBaud) + ' BAUD.')

    def readSerialStart(self):
        if self.thread == None:
            self.thread = Thread(target=self.backgroundThread)
            self.thread.start()
            # Block till we start receiving values
            while self.isReceiving != True:
                time.sleep(0.1)

    def getSerialData(self, frame, lines, lineValueText, lineLabel, timeText, pltNumber):
        if pltNumber == 0:  # in order to make all the clocks show the same reading
            currentTimer = time.perf_counter()
            self.plotTimer = int((currentTimer - self.previousTimer) * 1000)     # the first reading will be erroneous
            self.previousTimer = currentTimer
        self.privateData = copy.deepcopy(self.rawData)    # so that the 3 values in our plots will be synchronized to the same sample time
        timeText.set_text('Plot Interval = ' + str(self.plotTimer) + 'ms')
        data = self.privateData[(pltNumber*self.dataNumBytes):(self.dataNumBytes + pltNumber*self.dataNumBytes)]
        value,  = struct.unpack(self.dataType, data)
        self.data[pltNumber].append(value)    # we get the latest data point and append it to our array
        lines.set_data(range(self.plotMaxLength), self.data[pltNumber])
        lineValueText.set_text('[' + lineLabel + '] = ' + str(value))

    def backgroundThread(self):    # retrieve data
        time.sleep(1.0)  # give some buffer time for retrieving data
        self.serialConnection.reset_input_buffer()
        while (self.isRun):
            self.serialConnection.readinto(self.rawData)
            self.isReceiving = True

    def close(self):
        self.isRun = False
        self.thread.join()
        self.serialConnection.close()
        print('Disconnected...')
class Window(Frame):
    def __init__(self, figure, master, SerialReference,orient):
        Frame.__init__(self, master)
        self.entry = None
        self.setPoint = None
        self.master = master        # a reference to the master window
        self.serialReference = SerialReference      # keep a reference to our serial connection so that we can use it for bi-directional communicate from this class
        self.initWindow(figure,orient)     # initialize the window with our settings

    
    def initWindow(self, figure,orient):
        self.master.title("Real Time Plot")
        if orient == 0:
            canvas = FigureCanvasTkAgg(figure, master=self.master)
            toolbar = NavigationToolbar2Tk(canvas, self.master)
            canvas.get_tk_widget().pack(side=Tk.LEFT,padx=5,pady=5)
        if orient == 1:
            canvas1 = FigureCanvasTkAgg(figure, master=self.master)
            toolbar1 = NavigationToolbar2Tk(canvas1, self.master)
            canvas1.get_tk_widget().pack(side=Tk.LEFT,padx=5,pady=5)
        if orient == 2:
            canvas2 = FigureCanvasTkAgg(figure, master=self.master)
            toolbar2 = NavigationToolbar2Tk(canvas2, self.master)
            canvas2.get_tk_widget().pack(side=Tk.RIGHT,padx=5,pady=5)
        if orient == 3:
            canvas3 = FigureCanvasTkAgg(figure, master=self.master)
            toolbar3 = NavigationToolbar2Tk(canvas3, self.master)
            canvas3.get_tk_widget().pack(side=Tk.RIGHT,padx=5,pady=5)
        if orient == 4:
            canvas4 = FigureCanvasTkAgg(figure, master=self.master)
            toolbar4 = NavigationToolbar2Tk(canvas4, self.master)
            canvas4.get_tk_widget().pack(side=Tk.LEFT,padx=5,pady=5)

def makeFigure(xLimit, yLimit, title):
    xmin, xmax = xLimit
    ymin, ymax = yLimit
    fig = plt.figure()
    ax = plt.axes(xlim=(xmin, xmax), ylim=(int(ymin - (ymax - ymin) / 10), int(ymax + (ymax - ymin) / 10)))
    ax.set_title(title)
    ax.set_xlabel("Time")
    ax.set_ylabel("Accelerometer Output")
    return fig, ax


def main():
    portName = 'COM16'
    ##portName = '/dev/ttyUSB0'
    baudRate = 115200
    maxPlotLength = 1000     # number of points in x-axis of real time plot
    dataNumBytes = 2        # number of bytes of 1 data point
    numPlots = 5            # number of plots in 1 graph
    numGraphs = 1
    s = serialPlot(portName, baudRate, maxPlotLength, dataNumBytes, numPlots)   # initializes all required variables
    s.readSerialStart()                                               # starts background thread

    # plotting starts below
    pltInterval = 100    # Period at which the plot animation updates [ms]
    lineLabelText = ['Accel 1', 'Accel 2', 'Brake','Motor Temp','Motor RPM']
    title = ['Accel 1', 'Accel 2', 'Brake','Motor Temp','Motor RPM']
    xLimit = [(0, maxPlotLength), (0, maxPlotLength), (0, maxPlotLength),(0,maxPlotLength),(0,maxPlotLength)]
    yLimit = [(-20, 5000), (-20, 5000), (-20, 5000), (-20, 100),(-20,5000)]
    style = ['r-', 'c-', 'b-','g-','y-']  # linestyles for the different plots
    anim = []
    root=Tk.Tk()
    root2=Tk.Tk()
    for i in range(numPlots-2):
        fig, ax = makeFigure(xLimit[i], yLimit[i], title[i])
        lines = ax.plot([], [], style[i], label=lineLabelText[i])[0]
        timeText = ax.text(0.50, 0.95, '', transform=ax.transAxes)
        lineValueText = ax.text(0.50, 0.90, '', transform=ax.transAxes)
        anim.append(animation.FuncAnimation(fig, s.getSerialData, fargs=(lines, lineValueText, lineLabelText[i], timeText, i), interval=pltInterval))  # fargs has to be a tuple
        plt.legend(loc="upper left")
        apps=Window(fig,root,s,i)
    for i in range(3,numPlots):
        fig, ax = makeFigure(xLimit[i], yLimit[i], title[i])
        lines = ax.plot([], [], style[i], label=lineLabelText[i])[0]
        timeText = ax.text(0.50, 0.95, '', transform=ax.transAxes)
        lineValueText = ax.text(0.50, 0.90, '', transform=ax.transAxes)
        anim.append(animation.FuncAnimation(fig, s.getSerialData, fargs=(lines, lineValueText, lineLabelText[i], timeText, i), interval=pltInterval))  # fargs has to be a tuple
        plt.legend(loc="upper left")
        app=Window(fig,root2,s,i)

    ##plt.show()
    root.mainloop()
    s.close()


if __name__ == '__main__':
    main()