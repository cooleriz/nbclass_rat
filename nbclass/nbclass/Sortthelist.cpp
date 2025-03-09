#include"pch.h"
#include"Sortthelist.h"

int strsort_column;	// 记录点击的列
//排序
int CALLBACK CompareListStrProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	int row1 = (int)lParam1;
	int row2 = (int)lParam2;
	CListCtrl* lc = (CListCtrl*)lParamSort;
	CString lp1 = lc->GetItemText(row1, strsort_column);
	CString lp2 = lc->GetItemText(row2, strsort_column);
	return lp2.CompareNoCase(lp1);

}

int CALLBACK CompareListNumberProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	int row1 = (int)lParam1;
	int row2 = (int)lParam2;
	CListCtrl* lc = (CListCtrl*)lParamSort;
	CString lp1 = lc->GetItemText(row1, strsort_column);
	CString lp2 = lc->GetItemText(row2, strsort_column);
	

	return atoi(lp2) - atoi(lp1);
	
}