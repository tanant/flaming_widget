import PyQt4.QtGui as QtGui  
import PyQt4.QtCore as QtCore  
import math

def report():  
    print "QtGui.QStyle."+QtCore.QObject().sender().objectName()

SP_tokens = []  
for x in dir(QtGui.QStyle):  
    if x.startswith('SP'):
        SP_tokens.append(x)
rowsize = int(math.sqrt(len(SP_tokens)))

buttongrid = QtGui.QDialog()  
buttongrid.setLayout(QtGui.QGridLayout())

for idx, x in enumerate(SP_tokens):  
    button = QtGui.QPushButton()
    button.setToolTip(x)
    button.setIcon(QtGui.QApplication.style().standardIcon(getattr(QtGui.QStyle, x)))
    button.setObjectName(x)
    QtCore.QObject.connect(button, QtCore.SIGNAL('clicked()'), report)

    rowidx = idx//rowsize
    colidx = idx%rowsize

    buttongrid.layout().addWidget(button, rowidx, colidx)

buttongrid.show()  