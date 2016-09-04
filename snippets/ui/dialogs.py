import PyQt4.QtGui as QtGui
import PyQt4.QtCore as QtCore

class MessageDialog(QtGui.QDialog):
    """
    This class is an enhanced Dialog box that provides for a simple layout to display a title,
    a brief blurb, and a large (scrolling) freeform text area that can display detail, for 
    example, as a stack trace dump.
    
    In the standard case, an 'ok'/acknowledge button is provided, that on click is linked to 
    the QDialog's 'accept()' slot. If you want to have more buttons, then you must provide them
    yourself as constructed objects into the constructor, and deal with signal linking yourself.
    
    There are two simplified wrapper classes:
    for the common Yes/No case
    - 'yes' is plugged into the accept(), and no into the reject slots. 
    - You can provide alt text for the buttons. The default (enter) option is always leftmost
    
    for the multi-option case
    - a list of button labels is provided
    - all are pushed into an internal enumerate function, returning the index of the button
    """

    def __init__(self,
                title_text_string,    
                info_text_string,
                long_text_string="",

                icon_margin=20,
                icon_size=None,
                icon_pixmap=None,

                min_width=600,
                max_width=None,
                min_height=200,
                max_height=None,

                buttons=None,   # default is a staight OK button, otherwise feed in your custom buttons

                parent=None,

                stylesheet="""QLabel#title {font-weight:bold; color:#fa3}
                              QLabel#info  {font-weight:normal; }
                              QTextEdit#long  { }
                              QPushButton#defaultbutton {font-weight:bold}""",
                delete_on_close=True,
                ):
        """
        This is a simple expansion of the old nuke.message and nuke.ask convenience functions
        """
        super(MessageDialog,self).__init__(parent=parent)
    
        self.widgets = []
        self.buttons = []

        informational_image = QtGui.QLabel()
        if icon_pixmap is None:
           icon_pixmap = QtGui.QApplication.style().standardPixmap(QtGui.QStyle.SP_MessageBoxInformation)

        if icon_size is None:
            icon_image = icon_pixmap
        else:
            icon_image = icon_pixmap.scaledToWidth(icon_size)
        
        informational_image.setPixmap(icon_image)
        informational_image.setAlignment(QtCore.Qt.AlignTop)
        informational_image.setMargin(icon_margin)
        self.widgets.append( ('image',informational_image) )

        title_text = QtGui.QLabel()
        title_text.setWordWrap(True)
        title_text.setObjectName("title")
        title_text.setText(title_text_string)
        self.widgets.append( ('title',title_text) )

        info_text = QtGui.QLabel(self)
        info_text.setWordWrap(True)
        info_text.setObjectName("info")
        info_text.setText(info_text_string)
        self.widgets.append( ('info',info_text) )

        if stylesheet is None:
            pass
        else:
            pass

        main_layout = QtGui.QVBoxLayout(self)
        composite_header_layout = QtGui.QHBoxLayout()
        text_info_layout =  QtGui.QVBoxLayout()
        button_row_layout = QtGui.QHBoxLayout()

        if buttons is None:
            button = QtGui.QPushButton('OK', self)
            button.setIcon(QtGui.QApplication.style().standardIcon(QtGui.QStyle.SP_DialogOkButton))
            button.setMaximumWidth(100)
            self.buttons.append(button)
            QtCore.QObject.connect(button, QtCore.SIGNAL('clicked()'), self, QtCore.SLOT('accept()'))
        else:
            self.buttons.extend(buttons)       

        self.buttons[0].setDefault(True)
        self.buttons[0].setFocus(True)
        self.buttons[0].setObjectName("defaultbutton")
        
        button_row_layout.addItem(QtGui.QSpacerItem(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding))
        for button in self.buttons:
            button_row_layout.addWidget(button)
            button.setParent(self)  # take control of the generic buttons
        button_row_layout.addItem(QtGui.QSpacerItem(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding))
    
        composite_header_layout.addWidget(informational_image ,0)
        composite_header_layout.addLayout(text_info_layout ,1)
        main_layout.addLayout(composite_header_layout,0)
        
        text_info_layout.addWidget(title_text,0)
        text_info_layout.addWidget(info_text,1)
        main_layout.addLayout(button_row_layout,1)

        if long_text_string is not None:
            freeform_textfield = QtGui.QTextEdit(self)
            freeform_textfield.setObjectName("long")
            main_layout.addWidget(freeform_textfield ,2)
            freeform_textfield.setReadOnly(True)
            freeform_textfield.setTextInteractionFlags(freeform_textfield.textInteractionFlags() | QtCore.Qt.TextSelectableByMouse)
            freeform_textfield.setText(long_text_string)

        if min_width:
            self.setMinimumWidth(min_width)
        if max_width:
            self.setMaximumWidth(max_width)
        if min_height:
            self.setMinimumHeight(min_height)
        if max_width:
            self.setMaximumHeight(max_height)

        self.setStyleSheet(stylesheet)

        if delete_on_close:
            self.setAttribute(QtCore.Qt.WA_DeleteOnClose)

    def showModal(self):
        return self.exec_()


class MultiOptionDialog(MessageDialog):
    def __init__(self, 
                title_text_string,    
                info_text_string,
                long_text_string=None,
                button_text_list=["yes","no", "maybe"],
                icon_margin=20,
                icon_size=None,
                min_width=600,
                max_width=None,
                min_height=100,
                max_height=None,
                parent=None,
                ):
        
        super(MultiOptionDialog,self).__init__(     title_text_string=title_text_string,
                                            info_text_string=info_text_string,
                                            long_text_string=long_text_string,
                                            icon_margin=icon_margin,
                                            icon_size=icon_size,
                                            icon_pixmap=QtGui.QApplication.style().standardPixmap(QtGui.QStyle.SP_MessageBoxQuestion),
                                            min_width=min_width,
                                            max_width=max_width,
                                            min_height=min_height,
                                            max_height=max_height,
                                            buttons=[QtGui.QPushButton(item) for item in button_text_list],
                                            parent=parent)

        for button in self.buttons:
            QtCore.QObject.connect(button, QtCore.SIGNAL('clicked()'), self._enumerate_button)
     
    def _enumerate_button(self):
        self.done(self.buttons.index(self.sender()))


class AskDialog(MessageDialog):
    def __init__(self, 
                title_text_string,    
                info_text_string,
                long_text_string=None,

                icon_margin=20,
                icon_size=None,

                min_width=600,
                max_width=None,
                min_height=100,
                max_height=None,

                parent=None,

                yes_text = "yes",
                no_text = "no",
                default_yes = True
                ):

        no_button = QtGui.QPushButton(no_text)
        no_button.setIcon(QtGui.QApplication.style().standardIcon(QtGui.QStyle.SP_DialogNoButton))
        yes_button = QtGui.QPushButton(yes_text)
        yes_button.setIcon(QtGui.QApplication.style().standardIcon(QtGui.QStyle.SP_DialogYesButton))

        if default_yes:
            button_list = [yes_button, no_button]
        else:
            button_list = [no_button, yes_button]

        super(AskDialog,self).__init__(     title_text_string=title_text_string,
                                            info_text_string=info_text_string,
                                            long_text_string=long_text_string,
                                            icon_margin=icon_margin,
                                            icon_size=icon_size,
                                            icon_pixmap=QtGui.QApplication.style().standardPixmap(QtGui.QStyle.SP_MessageBoxQuestion),
                                            min_width=min_width,
                                            max_width=max_width,
                                            min_height=min_height,
                                            max_height=max_height,
                                            buttons=[no_button, yes_button],
                                            parent=parent)
        QtCore.QObject.connect(yes_button, QtCore.SIGNAL('clicked()'), self, QtCore.SLOT('accept()'))
        QtCore.QObject.connect(no_button , QtCore.SIGNAL('clicked()'), self, QtCore.SLOT('reject()'))
       


print AskDialog("Chickens","are they not awesome?").showModal()
print MultiOptionDialog("Potatoes", "what variety do you like?", button_text_list=["russet", "purple", "royal blue", "kwango", "sweet"]).showModal()
print MessageDialog("hello", 
                "This would be informational text, to describe a little bit about the error message", 
                "You could have a heap of data here.. code for example, or a nuke.nodepaste, or something", 
                icon_size=None,
).showModal() 