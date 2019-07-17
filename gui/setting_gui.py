# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'gui/setting_gui.ui'
#
# Created by: PyQt5 UI code generator 5.12.1
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_IPSettingUI(object):
    def setupUi(self, IPSettingUI):
        IPSettingUI.setObjectName("IPSettingUI")
        IPSettingUI.setEnabled(True)
        IPSettingUI.resize(400, 300)
        IPSettingUI.setMinimumSize(QtCore.QSize(400, 300))
        IPSettingUI.setMaximumSize(QtCore.QSize(400, 300))
        self.verticalLayoutWidget = QtWidgets.QWidget(IPSettingUI)
        self.verticalLayoutWidget.setGeometry(QtCore.QRect(0, 30, 401, 181))
        self.verticalLayoutWidget.setObjectName("verticalLayoutWidget")
        self.verticalLayout = QtWidgets.QVBoxLayout(self.verticalLayoutWidget)
        self.verticalLayout.setContentsMargins(0, 0, 0, 0)
        self.verticalLayout.setObjectName("verticalLayout")
        self.horizontalLayout = QtWidgets.QHBoxLayout()
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.label = QtWidgets.QLabel(self.verticalLayoutWidget)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label.sizePolicy().hasHeightForWidth())
        self.label.setSizePolicy(sizePolicy)
        self.label.setMaximumSize(QtCore.QSize(50, 16777215))
        self.label.setSizeIncrement(QtCore.QSize(10, 0))
        self.label.setLayoutDirection(QtCore.Qt.LeftToRight)
        self.label.setAlignment(QtCore.Qt.AlignRight|QtCore.Qt.AlignTrailing|QtCore.Qt.AlignVCenter)
        self.label.setObjectName("label")
        self.horizontalLayout.addWidget(self.label)
        self.editAddr = QtWidgets.QLineEdit(self.verticalLayoutWidget)
        self.editAddr.setMaximumSize(QtCore.QSize(200, 16777215))
        self.editAddr.setObjectName("editAddr")
        self.horizontalLayout.addWidget(self.editAddr)
        self.verticalLayout.addLayout(self.horizontalLayout)
        self.horizontalLayout_2 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")
        self.label_2 = QtWidgets.QLabel(self.verticalLayoutWidget)
        self.label_2.setMaximumSize(QtCore.QSize(50, 16777215))
        self.label_2.setAlignment(QtCore.Qt.AlignRight|QtCore.Qt.AlignTrailing|QtCore.Qt.AlignVCenter)
        self.label_2.setObjectName("label_2")
        self.horizontalLayout_2.addWidget(self.label_2)
        self.editPort = QtWidgets.QLineEdit(self.verticalLayoutWidget)
        self.editPort.setMaximumSize(QtCore.QSize(200, 16777215))
        self.editPort.setObjectName("editPort")
        self.horizontalLayout_2.addWidget(self.editPort)
        self.verticalLayout.addLayout(self.horizontalLayout_2)
        self.confirmBtn = QtWidgets.QPushButton(IPSettingUI)
        self.confirmBtn.setGeometry(QtCore.QRect(150, 230, 100, 50))
        self.confirmBtn.setMaximumSize(QtCore.QSize(100, 50))
        self.confirmBtn.setLayoutDirection(QtCore.Qt.LeftToRight)
        self.confirmBtn.setObjectName("confirmBtn")

        self.retranslateUi(IPSettingUI)
        QtCore.QMetaObject.connectSlotsByName(IPSettingUI)

    def retranslateUi(self, IPSettingUI):
        _translate = QtCore.QCoreApplication.translate
        IPSettingUI.setWindowTitle(_translate("IPSettingUI", "Form"))
        self.label.setText(_translate("IPSettingUI", "IP 주소"))
        self.label_2.setText(_translate("IPSettingUI", "포트"))
        self.confirmBtn.setText(_translate("IPSettingUI", "완료"))




if __name__ == "__main__":
    import sys
    app = QtWidgets.QApplication(sys.argv)
    IPSettingUI = QtWidgets.QWidget()
    ui = Ui_IPSettingUI()
    ui.setupUi(IPSettingUI)
    IPSettingUI.show()
    sys.exit(app.exec_())
