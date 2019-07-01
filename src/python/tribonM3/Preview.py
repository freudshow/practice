#Boa:Frame:Preview
import os
import kcs_ui
import kcs_draft
import kcs_dex
import kcs_util
from wxPython.wx import *

def create(parent):
    return Preview(parent)

[wxID_PREVIEWBUTTONCHANGEANDSAVE, wxID_PREVIEWBUTTONCLEARLIST, 
 wxID_PREVIEWBUTTONEDIT, wxID_PREVIEWBUTTONEXIT, wxID_PREVIEWBUTTONPREVIEW, 
 wxID_PREVIEWBUTTONSEARCH, wxID_PREVIEWBUTTONSEARCHBY, wxID_PREVIEWGAUGE, 
 wxID_PREVIEWLISTCTRLDWGLIST, wxID_PREVIEWPANEL1, wxID_PREVIEWPREVIEW, 
 wxID_PREVIEWRADIOBOXCHOOSEDWGTYPE, wxID_PREVIEWSTATICBOXCHANGEPAGE, 
 wxID_PREVIEWSTATICTEXTKEYIN, wxID_PREVIEWSTATICTEXTSEPERATOR, 
 wxID_PREVIEWSTATICTEXTSTX, wxID_PREVIEWTEXTCTRLKEYIN, 
 wxID_PREVIEWTEXTCTRLNEWPAGE, wxID_PREVIEWTEXTCTRLPREPAGE, 
 wxID_PREVIEWTEXTCTRLTOTALPAGE, 
] = map(lambda _init_ctrls: wxNewId(), range(20))

class Preview(wxFrame):
    def _init_coll_listCtrlDwgList_Columns(self, parent):
        # generated method, don't edit

        parent.InsertColumn(col=0, format=wxLIST_FORMAT_LEFT,
              heading='Drawing Name', width=-1)
        parent.InsertColumn(col=1, format=wxLIST_FORMAT_LEFT, heading='Page',
              width=-1)

    def _init_ctrls(self, prnt):
        # generated method, don't edit
        wxFrame.__init__(self, id=wxID_PREVIEW, name='Preview', parent=prnt,
              pos=wxPoint(600, 278), size=wxSize(362, 594),
              style=wxDEFAULT_FRAME_STYLE, title='Preview')
        self.SetClientSize(wxSize(354, 560))

        self.panel1 = wxPanel(id=wxID_PREVIEWPANEL1, name='panel1', parent=self,
              pos=wxPoint(0, 0), size=wxSize(354, 560), style=wxTAB_TRAVERSAL)

        self.radioBoxChooseDwgType = wxRadioBox(choices=['Assembly', 'Cutting',
              'Nest'], id=wxID_PREVIEWRADIOBOXCHOOSEDWGTYPE,
              label='Please choose the dwg type', majorDimension=1,
              name='radioBoxChooseDwgType', parent=self.panel1,
              point=wxPoint(16, 8), size=wxDefaultSize,
              style=wxRA_SPECIFY_ROWS)
        EVT_RADIOBOX(self.radioBoxChooseDwgType,
              wxID_PREVIEWRADIOBOXCHOOSEDWGTYPE,
              self.OnRadioBoxChooseDwgTypeRadiobox)

        self.staticTextKeyIn = wxStaticText(id=wxID_PREVIEWSTATICTEXTKEYIN,
              label='Key In', name='staticTextKeyIn', parent=self.panel1,
              pos=wxPoint(24, 64), size=wxSize(30, 13), style=0)

        self.textCtrlKeyIn = wxTextCtrl(id=wxID_PREVIEWTEXTCTRLKEYIN,
              name='textCtrlKeyIn', parent=self.panel1, pos=wxPoint(64, 64),
              size=wxSize(100, 21), style=0, value='')

        self.buttonSearch = wxButton(id=wxID_PREVIEWBUTTONSEARCH,
              label='Search', name='buttonSearch', parent=self.panel1,
              pos=wxPoint(176, 64), size=wxSize(75, 23), style=0)
        EVT_BUTTON(self.buttonSearch, wxID_PREVIEWBUTTONSEARCH,
              self.OnButtonSearchButton)

        self.buttonSearchBy = wxButton(id=wxID_PREVIEWBUTTONSEARCHBY,
              label='Search By', name='buttonSearchBy', parent=self.panel1,
              pos=wxPoint(264, 64), size=wxSize(75, 23), style=0)
        EVT_BUTTON(self.buttonSearchBy, wxID_PREVIEWBUTTONSEARCHBY,
              self.OnButtonSearchByButton)

        self.listCtrlDwgList = wxListCtrl(id=wxID_PREVIEWLISTCTRLDWGLIST,
              name='listCtrlDwgList', parent=self.panel1, pos=wxPoint(16, 104),
              size=wxSize(320, 304), style=wxLC_REPORT | wxLC_ICON)
        self._init_coll_listCtrlDwgList_Columns(self.listCtrlDwgList)
        EVT_LIST_ITEM_SELECTED(self.listCtrlDwgList,
              wxID_PREVIEWLISTCTRLDWGLIST,
              self.OnListCtrlDwgListListItemSelected)
        EVT_LIST_ITEM_ACTIVATED(self.listCtrlDwgList,
              wxID_PREVIEWLISTCTRLDWGLIST,
              self.OnListCtrlDwgListListItemActivated)

        self.gauge = wxGauge(id=wxID_PREVIEWGAUGE, name='gauge',
              parent=self.panel1, pos=wxPoint(16, 416), range=100,
              size=wxSize(320, 8), style=wxGA_HORIZONTAL)

        self.buttonClearList = wxButton(id=wxID_PREVIEWBUTTONCLEARLIST,
              label='Clear List', name='buttonClearList', parent=self.panel1,
              pos=wxPoint(16, 432), size=wxSize(75, 23), style=0)
        EVT_BUTTON(self.buttonClearList, wxID_PREVIEWBUTTONCLEARLIST,
              self.OnButtonClearListButton)

        self.buttonPreview = wxButton(id=wxID_PREVIEWBUTTONPREVIEW,
              label='Preview', name='buttonPreview', parent=self.panel1,
              pos=wxPoint(136, 432), size=wxSize(75, 23), style=0)
        EVT_BUTTON(self.buttonPreview, wxID_PREVIEWBUTTONPREVIEW,
              self.OnButtonPreviewButton)

        self.buttonEdit = wxButton(id=wxID_PREVIEWBUTTONEDIT, label='Edit',
              name='buttonEdit', parent=self.panel1, pos=wxPoint(256, 432),
              size=wxSize(75, 23), style=0)
        EVT_BUTTON(self.buttonEdit, wxID_PREVIEWBUTTONEDIT,
              self.OnButtonEditButton)

        self.staticBoxChangePage = wxStaticBox(id=wxID_PREVIEWSTATICBOXCHANGEPAGE,
              label='Change Page', name='staticBoxChangePage',
              parent=self.panel1, pos=wxPoint(16, 464), size=wxSize(320, 56),
              style=0)

        self.textCtrlPrePage = wxTextCtrl(id=wxID_PREVIEWTEXTCTRLPREPAGE,
              name='textCtrlPrePage', parent=self.panel1, pos=wxPoint(24, 488),
              size=wxSize(32, 21), style=0, value='')

        self.textCtrlNewPage = wxTextCtrl(id=wxID_PREVIEWTEXTCTRLNEWPAGE,
              name='textCtrlNewPage', parent=self.panel1, pos=wxPoint(96, 488),
              size=wxSize(32, 21), style=0, value='')

        self.staticTextSeperator = wxStaticText(id=wxID_PREVIEWSTATICTEXTSEPERATOR,
              label='/', name='staticTextSeperator', parent=self.panel1,
              pos=wxPoint(144, 488), size=wxSize(8, 16), style=0)

        self.textCtrlTotalPage = wxTextCtrl(id=wxID_PREVIEWTEXTCTRLTOTALPAGE,
              name='textCtrlTotalPage', parent=self.panel1, pos=wxPoint(160,
              488), size=wxSize(32, 21), style=0, value='')

        self.buttonChangeAndSave = wxButton(id=wxID_PREVIEWBUTTONCHANGEANDSAVE,
              label='Change and Save', name='buttonChangeAndSave',
              parent=self.panel1, pos=wxPoint(200, 488), size=wxSize(120, 23),
              style=0)
        EVT_BUTTON(self.buttonChangeAndSave, wxID_PREVIEWBUTTONCHANGEANDSAVE,
              self.OnButtonChangeAndSaveButton)

        self.buttonExit = wxButton(id=wxID_PREVIEWBUTTONEXIT, label='Exit',
              name='buttonExit', parent=self.panel1, pos=wxPoint(256, 528),
              size=wxSize(75, 23), style=0)
        EVT_BUTTON(self.buttonExit, wxID_PREVIEWBUTTONEXIT,
              self.OnButtonExitButton)

        self.staticTextSTX = wxStaticText(id=wxID_PREVIEWSTATICTEXTSTX,
              label='STX Ship Building Co.,Ltd', name='staticTextSTX',
              parent=self.panel1, pos=wxPoint(8, 536), size=wxSize(122, 13),
              style=0)

    def __init__(self, parent):
        self._init_ctrls(parent)
################################################################################
###                            My Functions                                  ###
################################################################################

    def GetDwgDB(self):
        select = self.RadioBoxChooseDwgType.GetSelection()
        dwg_DBs = ["SB_PDB","SB_PDB","SB_NPL"]
        return dwg_DBs[select]
#-------------------------------------------------------------------------------
        
    def GetDwgType(self):
        select = self.RadioBoxChooseDwgType.GetSelection()
        types  = ['A173*',self.GetCutVersion(),'*P']
        return types[select]
#-------------------------------------------------------------------------------
        
    def GetShipNo(self):
        proj = kcs_util.TB_environment_get("SB_PROJECT")
        No   = kcs_util.TB_environment_get("SBH_SHIP_NUMBER")
        return proj+No
#-------------------------------------------------------------------------------
        
    def GetCutVersion(self):
        current_ship = self.GetShipNo()
        old_ships = ["DA1001","DA1002","DA1003","DA1004"]
        if current_ship in old_ships:
            return "C1722*"
        else:
            return "C173*"
#-------------------------------------------------------------------------------
            
    def CreateInFile(self):
        stmt = self.GetInstmt()
        try:
            f = open("c:\Tribon\Temp\sa004.in.txt",'w')
            f.write(stmt)
            f.close()
        except IOError,e:
            kcs_ui.message_confirm(e)
#-------------------------------------------------------------------------------
            
    def GetInStmt(self):
        prefix = "4\n2\n"
        postfix = "\n\n\n\n\nN\n0\n"
        db = self.GetDwgDB()
        stmt = prefix+db+"\n\n"+self.GetDwgType()+postfix
        return stmt
#-------------------------------------------------------------------------------
        
    def CreateOutFile(self):
        self.CreateInFile()
        run_stmt = "C:\Tribon\M3\Bin\sa004.exe<C:\Tribon\Temp\sa004.in.txt>C:\Tribon\Temp\sa004.out.txt"
        try:
            f = os.popen(run_stmt)
            f.close()
        except IOError,e:
            kcs_ui.message_confirm(e)
#-------------------------------------------------------------------------------
    
    def GetPieceList(self,dwg_list,dwg):
        split = dwg.split('-')
        piece_list = []
        for i in dwg_list:
            if split[1] in i:
                piece_list.append(i)
        return piece_list
#-------------------------------------------------------------------------------
    
    def GetDwgNameList(self):
        self.CreateOutFile()
        outfile = "C:\\Tribon\\temp\\sa004.out.txt"
        f = open(outfile,'r')
        list = f.readlines()
        f.close()
        list = list[6:len(list)-2]
        if list == []:
            kcs_ui.message_confirm('drawing not fount')
            return
        dwg_name_list = []
        for i in list:
            split = i.split(' ')
            dwg_name_list.append(split[1])
        select = self.radioBoxChooseDwgType.GetSelection()
        if select == 1:
            return self.GetCutList(dwg_name_list)
        else:
            return dwg_name_list
#-------------------------------------------------------------------------------
            
    def GetCutList(self,dwg_list):
        cut_types = ["CBS","CSR","CEY","CFP","CSS","CSP","CFB","CBT"]
        cut_list = []
        for i in dwg_list:
            for j in cut_types:
                if j in i:
                    cut_list.append(i)
                    break
        return cut_list
#-------------------------------------------------------------------------------
        
    def GetColumnText(self,index,col):
        item = self.listCtrlDwgList.GetItem(index,col)
        return item.GetText()
#-------------------------------------------------------------------------------
    
    def GetPageNo(self,dwg_name,rule):
        stmt = "DRA('"+dwg_name+'@'+self.GetDwgDB()+"').TEXT_BY_RULE("+str(rule)+").ROW(1)"
        kcs_dex.extract(stmt)
        if kcs_dex.next_result() == 3:
            page_number = kcs_dex.get_string()
        else:
            page_number = ' '
        return page_number
#-------------------------------------------------------------------------------
        
    def GetPageList(self,dwg_list):
        self.gauge.SetValue(0)
        self.gauge.SetRange(len(dwg_list))
        page_list = []
        for i,item in enumerate(dwg_list):
            self.gauge.SetValue(i)
            page_list.append(self.GetPageNo(item,8888))
        return page_list
#-------------------------------------------------------------------------------
        
    def GetCfbPageList(self,cfb_list):
        cfb_page_list = [[],[]]
        for i,item in enumerate(cfb_list):
            stmt = "DRA('"+item+"').VIEW(*).TEXT(75:76).VALUE"
            kcs_dex.extract(stmt)
            if kcs_dex.next_result() == 3:
                cfb_page_list[0].append(kcs_dex.get_string())
            else:
                cfb_page_list[0].append(' ')
            if kcs_dex.next_result() == 3:
                cfb_page_list[1].append(kcs_dex.get_string())
            else:
                cfb_page_list[1].append(' ')
        return cfb_page_list
#-------------------------------------------------------------------------------

    def SetListCtrl(self,dwg_list,page_list):
        self.gauge.SetValue(0)
        self.gauge.SetRange(len(dwg_list))
        for i,item in enumerate(dwg_list):
            self.gauge.SetValue(i)
            self.listCtrlDwgList.InsertStringItem(i,' ')
            self.listCtrlDwgList.SetStringItem(i,0,item)
            self.listCtrlDwgList.SetStringItem(i,0,page_list[i])
        self.listCtrlDwgList.SetColumnWidth(0,wxLIST_AUTOSIZE)
        self.listCtrlDwgList.SetColumnWidth(1,60)
        self.gauge.SetValue(len(dwg_list))
#-------------------------------------------------------------------------------
        
    def OpenCurrentDwg(self,dwg,mode):
        if kcs_draft.dwg_current():
            kcs_draft.dwg_close()
        try:
            kcs_draft.dwg_open(dwg,self.GetDwgDB(),mode)
        except:
            error = kcs_draft.error
            self.KcsDraftErrorDict = {
                                "kcs_Error":"General error",
                                "kcs_DbError":"Databank error",
                                "kcs_DrawingCurrent":"A drawing is already current",
                                "kcs_DrawingLocked":"The drawing is locked in the databank.",
                                "kcs_DrawingNotFound":"The drawing was not found in the databank. ",
                                "kcs_NameOccupied":"The name is used by system.",
                                "kcs_VolumeCurrent":"The volume mode is active.",
                                "Kcs_RevisionNotFound":"The revision not found for given drawing.",
                                "Kcs_CouldNotOpenDatabank":"Not possible to open databank. \n\
                                            Probably because of wrong user database name or id."
                                    }
            kcs_ui.message_confirm(self.KcsDraftErrorDict[error])
#-------------------------------------------------------------------------------
        
    def GetMatchList(self,dwg_list,keyin):
        split = self.GetSplitKeyIn(keyin)
        for i,item in enumerate(dwg_list):
            for j in split:
                if j not in item:
                    dwg_list[i] = "DELETE"
                    break
        match_list = []
        for i in dwg_list:
            if i != "DELETE":
                match_list.append(i)
        return match_list
#-------------------------------------------------------------------------------
    
    def GetSplitKeyIn(self,keyin):
        keyin = keyin.upper()
        if keyin[0] == '*':
            keyin = keyin[1:len(keyin)]
        if keyin[len(keyin)-1] == '*':
            keyin = keyin[0:len(keyin)-1]
        return keyin.split('*')
#-------------------------------------------------------------------------------

################################################################################
###                          My Functions end                                ###
################################################################################

################################################################################
###                          Event Functions                                 ###
################################################################################

    def OnRadioBoxChooseDwgTypeRadiobox(self, event):
        self.ListCtrlDwgList.DeleteAllItems()
        self.dwg_list  = self.GetDwgNameList()
        self.Page_list = self.GetPageList(self.dwg_list)
        self.SetListCtrl(self.dwg_list,self.Page_list)
#-------------------------------------------------------------------------------

    def OnButtonSearchButton(self, event):
        self.ListCtrlDwgList.DeleteAllItems()
        keyIn      = self.textCtrlKeyIn.GetValue()
        match_list = self.GetMatchList(self.dwg_list,keyIn)
        match_page = self.GetPageList(match_list)
        self.SetListCtrl(match_list,match_page)
#-------------------------------------------------------------------------------

    def OnButtonSearchByButton(self, event):
        self.ListCtrlDwgList.DeleteAllItems()
        piece_list = self.GetPieceList(self.dwg_list,self.current_dwg)
        page_list  = self.GetPageList(piece_list)
        self.SetListCtrl(piece_list,page_list)
#-------------------------------------------------------------------------------
        
    def OnListCtrlDwgListListItemSelected(self, event):
        self.current_item  = event.m_itemIndex
        self.current_dwg   = self.GetColumnText(self.current_item,0)
        self.current_page  = self.GetPageNo(self.current_dwg,8888)
        self.current_Total = self.GetPageNo(self.current_dwg,8889)
        self.textCtrlPrePage.SetValue(self.current_page)
        self.textCtrlNewPage.SetValue(self.current_page)
        self.textCtrlTotalPage.SetValue(self.current_Total)
#-------------------------------------------------------------------------------
        
    def OnListCtrlDwgListListItemActivated(self, event):
        self.OpenCurrentDwg(self.current_dwg,kcs_draft.kcsOPENMODE_READONLY)
#-------------------------------------------------------------------------------

    def OnButtonClearListButton(self, event):
        self.ListCtrlDwgList.DeleteAllItems()
#-------------------------------------------------------------------------------

    def OnButtonPreviewButton(self, event):
        self.OpenCurrentDwg(self.current_dwg,kcs_draft.kcsOPENMODE_READONLY)
#-------------------------------------------------------------------------------

    def OnButtonEditButton(self, event):
        self.OpenCurrentDwg(self.current_dwg,kcs_draft.kcsOPENMODE_READWRITE)
#-------------------------------------------------------------------------------

    def OnButtonChangeAndSaveButton(self, event):
        if kcs_draft.dwg_current():
            kcs_draft.dwg_close()
        self.OpenCurrentDwg(self.current_dwg,kcs_draft.kcsOPENMODE_READWRITE)
        new_page  = self.textCtrlNewPage.GetValue()
        new_total = self.textCtrlTotalPage.GetValue()
        try:
            if new_page != "" and new_page != self.current_page:
                kcs_draft.rule_text_new(new_page,8888)
                self.current_page = new_page
                self.textCtrlPrePage.SetValue(self.current_page)
            if new_total != "" and new_total != self.current_total:
                kcs_draft.rule_text_new(new_total,8889)
                self.current_total = new_total
                self.textCtrlTotalPage.SetValue(self.current_total)
            if kcs_draft.dwg_current():
                kcs_draft.dwg_save()
        except:
            self.TextRuleErrorDict = {
            "kcs_DrawingNotCurrent":"No drawing was current",
            "kcs_NoFormInDrawing":"The drawing contains no form",
            "kcs_RuleNoInvalid":"Invalid rule (not found in form)",
            "kcs_FormNotFound":"The drawing form was not found in the data bank"
            }
            error = kcs_draft.error
            kcs_ui.message_confirm(self.TextRuleErrorDict[error])
#-------------------------------------------------------------------------------
        
    def OnButtonExitButton(self, event):
        self.Destroy()
################################################################################
###                          Event Functions end                             ###
################################################################################


class BoaApp(wxApp):
    def OnInit(self):
        wxInitAllImageHandlers()
        self.main = create(None)
        self.main.Show()
        self.SetTopWindow(self.main)
        return True

def main():
    application = BoaApp(0)
    application.MainLoop()

if __name__ == '__main__':
    main()





















