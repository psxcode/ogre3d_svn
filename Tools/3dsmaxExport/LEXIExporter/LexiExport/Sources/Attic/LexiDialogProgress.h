/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Mark Folkenberg,
Bo Krohn

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/

#ifndef __NDS_LexiExporter_ExportProgressDialog__
#define __NDS_LexiExporter_ExportProgressDialog__

#include <richedit.h>

//

class CExportProgressDlg : public GDI::Dialog
{
public:
	CExportProgressDlg(Window* pParent);
	~CExportProgressDlg();

	// Initialize global stepping
	void	InitGlobal(int iObjectCount);

	// Do a global step (once per ExportObject)
	void	GlobalStep();

	// Initialize local progress
	void	InitLocal(int iStepCount);

	// Do a local step
	void	LocalStep(const char *pszDesc=NULL);

	// Signal all export done
	void	ExportDone(void);

	// Check if we want to abort current export
	bool	CheckAbort();

protected:
	void	OnInitDialog();	
	void	OnAbort();

private:	
	static INT_PTR CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);	

	// text buffer for global info
	char	m_InfoBuffer[1024];
	int		m_iGlobalProgress;
	int		m_iTotalGlobal;
	int		m_iLocalProgress;

	bool	m_bAbortRequest;
	bool	m_bAbortFlag;

	GDI::Window*	m_pGlobalInfo;
	GDI::Window*	m_pGlobalProgress;
	GDI::Window*	m_pLocalInfo;
	GDI::Window*	m_pLocalProgress;
};

//

#endif // __NDS_LexiExporter_ExportProgressDialog__