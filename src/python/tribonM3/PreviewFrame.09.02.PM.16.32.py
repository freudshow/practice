#####################################################################
#   This module list the drawings according to the inputed key word #
#   change the page nubmer of the drawing requested                 #
#   preview or edit the drawing                                     #
#####################################################################

import kcs_ui
import kcs_dex
import kcs_util
import kcs_draft
import KcsObjectCriteria
from KcsObjectCriteria import ObjectCriteria
from wxPython.wx import *
from wxPython.grid import *

def create(parent):
	return wxFramePreview(parent)

[wxID_WXFRAMEPREVIEW, wxID_WXFRAMEPREVIEWBUTTONCHANGEPAGENO, 
wxID_WXFRAMEPREVIEWBUTTONCLEARLIST, wxID_WXFRAMEPREVIEWBUTTONDWGEDIT, 
wxID_WXFRAMEPREVIEWBUTTONDWGPREVIEW, wxID_WXFRAMEPREVIEWBUTTONEXIT, 
wxID_WXFRAMEPREVIEWBUTTONSAVEDWG, wxID_WXFRAMEPREVIEWBUTTONSEARCHBYBLOCK, 
wxID_WXFRAMEPREVIEWBUTTONSEARCHBYPIECE, wxID_WXFRAMEPREVIEWGAUGE, 
wxID_WXFRAMEPREVIEWLISTCTRLDWGLIST, wxID_WXFRAMEPREVIEWPANEL1, 
wxID_WXFRAMEPREVIEWRADIOBOXCHOOSEDWGTYPE, 
wxID_WXFRAMEPREVIEWSTATICBOXCHANGEPAGE, wxID_WXFRAMEPREVIEWSTATICTEXTKEYIN, 
wxID_WXFRAMEPREVIEWSTATICTEXTNEWPAGE, wxID_WXFRAMEPREVIEWSTATICTEXTSEPARATOR, 
wxID_WXFRAMEPREVIEWTEXTCTRLKEYIN, wxID_WXFRAMEPREVIEWTEXTCTRLNEWPAGE, 
wxID_WXFRAMEPREVIEWTEXTCTRLNEWTOTALPAGE, 
wxID_WXFRAMEPREVIEWTEXTCTRLPAGEPREVIEW, 
] = map(lambda _init_ctrls: wxNewId(), range(21))

class wxFramePreview(wxFrame):
	def _init_coll_listCtrlDwgList_Columns(self, parent):
		# generated method, don't edit

		parent.InsertColumn(col=0, format=wxLIST_FORMAT_LEFT,
			  heading='Dwg Name', width=-1)
		parent.InsertColumn(col=1, format=wxLIST_FORMAT_LEFT, heading='Page',
			  width=-1)

	def _init_ctrls(self, prnt):
		# generated method, don't edit
		wxFrame.__init__(self, id=wxID_WXFRAMEPREVIEW, name='wxFramePreview',
			  parent=prnt, pos=wxPoint(517, 298), size=wxSize(463, 601),
			  style=wxDEFAULT_FRAME_STYLE, title='Preview')
		self.SetClientSize(wxSize(455, 574))

		self.panel1 = wxPanel(id=wxID_WXFRAMEPREVIEWPANEL1, name='panel1',
			  parent=self, pos=wxPoint(0, 0), size=wxSize(455, 574),
			  style=wxTAB_TRAVERSAL)

		self.radioBoxChooseDwgType = wxRadioBox(choices=['Assembly', 'Cutting',
			  'Nesting'], id=wxID_WXFRAMEPREVIEWRADIOBOXCHOOSEDWGTYPE,
			  label='Choose Dwg Type', majorDimension=1,
			  name='radioBoxChooseDwgType', parent=self.panel1, point=wxPoint(8,
			  8), size=wxDefaultSize, style=wxRA_SPECIFY_ROWS)

		self.buttonSearchByBlock = wxButton(id=wxID_WXFRAMEPREVIEWBUTTONSEARCHBYBLOCK,
			  label='Search', name='buttonSearchByBlock', parent=self.panel1,
			  pos=wxPoint(232, 56), size=wxSize(75, 23), style=0)
		EVT_BUTTON(self.buttonSearchByBlock,
			  wxID_WXFRAMEPREVIEWBUTTONSEARCHBYBLOCK,
			  self.OnButtonSearchByBlockButton)

		self.buttonSearchByPiece = wxButton(id=wxID_WXFRAMEPREVIEWBUTTONSEARCHBYPIECE,
			  label='Search By', name='buttonSearchByPiece', parent=self.panel1,
			  pos=wxPoint(328, 56), size=wxSize(75, 23), style=0)
		EVT_BUTTON(self.buttonSearchByPiece,
			  wxID_WXFRAMEPREVIEWBUTTONSEARCHBYPIECE,
			  self.OnButtonSearchByPieceButton)

		self.staticTextKeyIn = wxStaticText(id=wxID_WXFRAMEPREVIEWSTATICTEXTKEYIN,
			  label='Key In', name='staticTextKeyIn', parent=self.panel1,
			  pos=wxPoint(8, 64), size=wxSize(30, 13), style=0)

		self.textCtrlKeyIn = wxTextCtrl(id=wxID_WXFRAMEPREVIEWTEXTCTRLKEYIN,
			  name='textCtrlKeyIn', parent=self.panel1, pos=wxPoint(56, 56),
			  size=wxSize(160, 21), style=0, value='')

		self.listCtrlDwgList = wxListCtrl(id=wxID_WXFRAMEPREVIEWLISTCTRLDWGLIST,
			  name='listCtrlDwgList', parent=self.panel1, pos=wxPoint(16, 96),
			  size=wxSize(424, 248), style=wxLC_REPORT | wxLC_ICON)
		self._init_coll_listCtrlDwgList_Columns(self.listCtrlDwgList)
		EVT_LIST_ITEM_SELECTED(self.listCtrlDwgList,
			  wxID_WXFRAMEPREVIEWLISTCTRLDWGLIST,
			  self.OnListCtrlDwgListListItemSelected)

		self.gauge = wxGauge(id=wxID_WXFRAMEPREVIEWGAUGE, name='gauge',
			  parent=self.panel1, pos=wxPoint(16, 360), range=100,
			  size=wxSize(424, 16), style=wxGA_HORIZONTAL)

		self.buttonClearList = wxButton(id=wxID_WXFRAMEPREVIEWBUTTONCLEARLIST,
			  label='Clear List', name='buttonClearList', parent=self.panel1,
			  pos=wxPoint(24, 384), size=wxSize(75, 23), style=0)
		EVT_BUTTON(self.buttonClearList, wxID_WXFRAMEPREVIEWBUTTONCLEARLIST,
			  self.OnButtonClearListButton)

		self.buttonDwgPreview = wxButton(id=wxID_WXFRAMEPREVIEWBUTTONDWGPREVIEW,
			  label='Preview', name='buttonDwgPreview', parent=self.panel1,
			  pos=wxPoint(184, 384), size=wxSize(75, 23), style=0)
		EVT_BUTTON(self.buttonDwgPreview, wxID_WXFRAMEPREVIEWBUTTONDWGPREVIEW,
			  self.OnButtonDwgPreviewButton)

		self.buttonDwgEdit = wxButton(id=wxID_WXFRAMEPREVIEWBUTTONDWGEDIT,
			  label='Edit', name='buttonDwgEdit', parent=self.panel1,
			  pos=wxPoint(360, 384), size=wxSize(75, 23), style=0)
		EVT_BUTTON(self.buttonDwgEdit, wxID_WXFRAMEPREVIEWBUTTONDWGEDIT,
			  self.OnButtonDwgEditButton)

		self.staticBoxChangePage = wxStaticBox(id=wxID_WXFRAMEPREVIEWSTATICBOXCHANGEPAGE,
			  label='Change Page', name='staticBoxChangePage',
			  parent=self.panel1, pos=wxPoint(24, 432), size=wxSize(416, 100),
			  style=0)

		self.buttonExit = wxButton(id=wxID_WXFRAMEPREVIEWBUTTONEXIT,
			  label='Exit', name='buttonExit', parent=self.panel1,
			  pos=wxPoint(360, 544), size=wxSize(75, 23), style=0)
		EVT_BUTTON(self.buttonExit, wxID_WXFRAMEPREVIEWBUTTONEXIT,
			  self.OnButtonExitButton)

		self.textCtrlPagePreview = wxTextCtrl(id=wxID_WXFRAMEPREVIEWTEXTCTRLPAGEPREVIEW,
			  name='textCtrlPagePreview', parent=self.panel1, pos=wxPoint(48,
			  472), size=wxSize(64, 21), style=0, value='')

		self.textCtrlNewPage = wxTextCtrl(id=wxID_WXFRAMEPREVIEWTEXTCTRLNEWPAGE,
			  name='textCtrlNewPage', parent=self.panel1, pos=wxPoint(160, 472),
			  size=wxSize(40, 21), style=0, value='')

		self.textCtrlNewTotalPage = wxTextCtrl(id=wxID_WXFRAMEPREVIEWTEXTCTRLNEWTOTALPAGE,
			  name='textCtrlNewTotalPage', parent=self.panel1, pos=wxPoint(224,
			  472), size=wxSize(48, 21), style=0, value='')

		self.staticTextNewPage = wxStaticText(id=wxID_WXFRAMEPREVIEWSTATICTEXTNEWPAGE,
			  label='New', name='staticTextNewPage', parent=self.panel1,
			  pos=wxPoint(128, 472), size=wxSize(22, 13), style=0)

		self.staticTextSeparator = wxStaticText(id=wxID_WXFRAMEPREVIEWSTATICTEXTSEPARATOR,
			  label='/', name='staticTextSeparator', parent=self.panel1,
			  pos=wxPoint(208, 472), size=wxSize(8, 21), style=0)

		self.buttonChangePageNo = wxButton(id=wxID_WXFRAMEPREVIEWBUTTONCHANGEPAGENO,
			  label='Change', name='buttonChangePageNo', parent=self.panel1,
			  pos=wxPoint(280, 472), size=wxSize(56, 23), style=0)
		EVT_BUTTON(self.buttonChangePageNo,
			  wxID_WXFRAMEPREVIEWBUTTONCHANGEPAGENO,
			  self.OnButtonChangePageNoButton)

		self.buttonSaveDwg = wxButton(id=wxID_WXFRAMEPREVIEWBUTTONSAVEDWG,
			  label='Save', name='buttonSaveDwg', parent=self.panel1,
			  pos=wxPoint(360, 472), size=wxSize(56, 23), style=0)
		EVT_BUTTON(self.buttonSaveDwg, wxID_WXFRAMEPREVIEWBUTTONSAVEDWG,
			  self.OnButtonSaveDwgButton)

	def __init__(self, parent):
		self._init_ctrls(parent)

################################################################################
#                          my own function start                               #
################################################################################
	def GetDwgType(self):
		dwg_type_list = ["Assembly drawing","General drawing","Hull nesting sketch"]
		#dwg_DB_list = ["SB_ASSPDB","SB_PDB","SB_NPL"]
		selection 	  = self.radioBoxChooseDwgType.GetSelection()
		dwg_type 	  = dwg_type_list[selection]
		return dwg_type
		
#-------------------------------------------------------------------------------
	def GetDwgDB(self,dwg_type):
		try:
			dict 		= kcs_draft.dwg_type_list_get() #dict is a dictionary of drawing type list
			dwg_dbNames = map(lambda x : x[0],dict.values())
			dwg_DBs 	= map(lambda x : x[1],dict.values())
			DB_keys 	= dict.keys()
			i = 0
			while i < len(dwg_DBs):
				if dwg_dbNames[i] == dwg_type:
					return dwg_DBs[i],DB_keys[i]
				i+=1
			if i == len(dwg_DBs):
				kcs_ui.message_confirm("drawing type do not exist in current project")
		except:
				kcs_ui.message_confirm(kcs_draft.error)

#-------------------------------------------------------------------------------
	def GetStmt(self):
		dwg_type = self.GetDwgType()
		text_keyin = self.textCtrlKeyIn.GetValue().upper()
		temp = text_keyin
		if temp[0]=='*':
			temp=temp[1:len(temp)]
		if temp[len(temp)-1]=='*':
			temp=temp[0:len(temp)-1]
		split=temp.split('*')
		str=""
		for i in split:
			str += i+"'*'"
		str=str[0:len(str)-3]
		
		prefix = text_keyin[0]
		postfix = text_keyin[len(text_keyin)-1]
		if prefix != '*' and postfix != '*':
			stmt = "DRA('"+str+'@'+dwg_type+"').NAM"
		elif prefix != '*' and postfix == '*':
			stmt = "DRA('"+str+'@'+dwg_type+"'*).NAM"
		elif prefix == '*' and postfix != '*':
			stmt = "DRA(*'"+str+'@'+dwg_type+"').NAM"
		else:
			stmt = "DRA(*'"+str+'@'+dwg_type+"'*).NAM"
		return stmt

#-------------------------------------------------------------------------------
	def GetDwgNameList(self,stmt_get_dwgname):
		self.gauge.SetRange(50)
		dwg_name_list = []
		try:
			if kcs_dex.extract(stmt_get_dwgname) == 0 :
				dataType = kcs_dex.next_result()
				if dataType < 0 :
					kcs_ui.message_confirm("drawngs not found")
					return []
				gauge = 0
				while dataType >= 0:
					if dataType == 3:
						if gauge == 50:
							gauge = 0
						gauge += 1
						self.gauge.SetValue(gauge)
						dwg_name_list.append(kcs_dex.get_string())
						dataType = kcs_dex.next_result()
			self.gauge.SetRange(gauge)
			self.gauge.SetValue(gauge)
			return dwg_name_list
		except:
			kcs_ui.message_confirm(kcs_dex.error)

#-------------------------------------------------------------------------------
	def GetColumnText(self,index,col):
		item = self.listCtrlDwgList.GetItem(index,col)
		return item.GetText()

#-------------------------------------------------------------------------------
	def GetTotalNumber(self):
		dwg_type 		 = self.GetDwgType()
		stmt_get_totalNo = "DRA('"+self.dwg_name+'@'+dwg_type+"').TEXT_BY_RULE(8889).ROW(1)"
		kcs_dex.extract(stmt_get_totalNo)
		if kcs_dex.next_result() < 0 :
			dwg_total_no = " "
		else :
			dwg_total_no = kcs_dex.get_string()
		return dwg_total_no

#-------------------------------------------------------------------------------
	def GetPageNoList(self,dwg_name_list):
		self.gauge.SetRange(50)
		gauge 	 = 0
		dwg_type = self.GetDwgType()
		dwg_pageNo_list = []
		for i in dwg_name_list :
			kcs_dex.extract("DRA('"+i+'@'+dwg_type+"').TEXT_BY_RULE(8888).ROW(1)")
			if gauge == 50:
				gauge = 0
			gauge += 1
			self.gauge.SetValue(gauge)
			if kcs_dex.next_result() < 0 :
				dwg_pageNo_list.append(" ")
			else :
				dwg_pageNo_list.append(kcs_dex.get_string())
		self.gauge.SetRange(gauge)
		self.gauge.SetValue(gauge)
		return dwg_pageNo_list

#-------------------------------------------------------------------------------
	def SetListCtrl(self,dwg_name_list,dwg_pageNo_list):
		for i in range(len(dwg_name_list)):
			self.listCtrlDwgList.InsertStringItem(i,'')
			self.listCtrlDwgList.SetStringItem(i,0,dwg_name_list[i])
			self.listCtrlDwgList.SetStringItem(i,1,dwg_pageNo_list[i])

#-------------------------------------------------------------------------------
	def OpenCurrentDwg(self,mode):
		if kcs_draft.dwg_current():
			kcs_draft.dwg_close()
		dwg_type = self.GetDwgType()
		DB,DBKey = self.GetDwgDB(dwg_type)
		try:
			kcs_draft.dwg_open(self.dwg_name,DB,mode)
		except:
			kcs_ui.message_confirm(kcs_draft.error)

################################################################################
#                          my own functions end                                #
################################################################################

################################################################################
#                          Event functions start                               #
################################################################################

	def OnButtonSearchByBlockButton(self, event):
		self.listCtrlDwgList.DeleteAllItems()
		# stmt_get_dwgname = self.GetDwgNameStmt()
		stmt_get_dwgname = self.GetStmt()
		dwg_name_list 	 = self.GetDwgNameList(stmt_get_dwgname)
		dwg_PangeNo_list = self.GetPageNoList(dwg_name_list)
		self.SetListCtrl(dwg_name_list,dwg_PangeNo_list)

#-------------------------------------------------------------------------------
	def OnButtonSearchByPieceButton(self, event):
		self.listCtrlDwgList.DeleteAllItems()
		stmt_piece_list 	= self.GetStmt()
		dwg_piece_list		= self.GetDwgNameList(stmt_piece_list)
		dwg_piece_page_list = self.GetPageNoList(dwg_piece_list)
		self.SetListCtrl(dwg_piece_list,dwg_piece_page_list)

#-------------------------------------------------------------------------------
	def OnListCtrlDwgListListItemSelected(self, event):
		current_item  = event.m_itemIndex
		self.dwg_name = self.GetColumnText(current_item,0)
		dwg_page 	  = self.GetColumnText(current_item,1)
		self.textCtrlPagePreview.SetValue(dwg_page)
		self.textCtrlNewPage.SetValue(dwg_page)
		dwg_total_No  = self.GetTotalNumber()
		self.textCtrlNewTotalPage.SetValue(dwg_total_No)

#-------------------------------------------------------------------------------
	def OnButtonClearListButton(self, event):
		self.listCtrlDwgList.DeleteAllItems()

#-------------------------------------------------------------------------------
	def OnButtonDwgPreviewButton(self, event):
		self.OpenCurrentDwg(kcs_draft.kcsOPENMODE_READONLY)

#-------------------------------------------------------------------------------
	def OnButtonDwgEditButton(self, event):
		self.OpenCurrentDwg(kcs_draft.kcsOPENMODE_READWRITE)
		self.Destroy()

#-------------------------------------------------------------------------------
	def OnButtonExitButton(self, event):
		if kcs_draft.dwg_current():
			ref=kcs_ui.answer_req('Save Current Drawing', 'OK to save current drawing before closing?')
			if ref == kcs_util.yes():
				try:
					kcs_draft.dwg_save()
				except:
					kcs_ui.message_confirm(kcs_draft.error)
		self.Destroy()

#-------------------------------------------------------------------------------
	def OnButtonChangePageNoButton(self, event):
		self.OpenCurrentDwg(kcs_draft.kcsOPENMODE_READWRITE)
		page_No 	= self.textCtrlPagePreview.GetValue()
		new_page_No = self.textCtrlNewPage.GetValue()
		if new_page_No != "" and page_No != new_page_No:
			try:
				kcs_draft.rule_text_new(new_page_No,8888)
			except:
				kcs_ui.message_confirm(kcs_draft.error)
		new_total_page 	  = self.textCtrlNewTotalPage.GetValue()
		previous_total_No = self.GetTotalNumber()
		if new_total_page != "" and previous_total_No != new_total_page :
			try:
				kcs_draft.rule_text_new(new_total_page,8889)
			except:
				kcs_ui.message_confirm(kcs_draft.error)

#-------------------------------------------------------------------------------
	def OnButtonSaveDwgButton(self, event):
		try:
			kcs_draft.dwg_save()
		except:
			kcs_ui.message_confirm(kcs_draft.error)

################################################################################
#                          Event functions end                                 #
################################################################################

################################################################################
#                          main function start                                 #
################################################################################
 
class PreviewApp(wxApp):
	def OnInit(self):
		wxInitAllImageHandlers()
		self.main = create(None)
		self.main.Show()
		self.SetTopWindow(self.main)
		return True

def main():
	application = PreviewApp(0)
	application.MainLoop()

if __name__ == '__main__':
	main()
################################################################################
#                         main function ends                                   #
################################################################################