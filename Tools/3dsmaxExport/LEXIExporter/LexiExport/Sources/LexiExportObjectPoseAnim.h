/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Mark Folkenberg

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

#ifndef __NDS_LexiExporter_ExportObject_PoseAnim__
#define __NDS_LexiExporter_ExportObject_PoseAnim__

#include "..\res\resource.h"
#include "LexiExportSupportTypes.h"
//

class CPoseAnimExportObject : public CExportObject 
{
public:
	// Constructor/Destructor
	CPoseAnimExportObject(CDDObject *pConfig);
	~CPoseAnimExportObject();

	// Get window for editing ExportObject properties
	GDI::Window* GetEditWindow(GDI::Window *pParent);

	// Close edit window
	void CloseEditWindow();

	// Called when object is first created [by user].
	// This allows for wizard-style editing of required data
	// If this function returns false, the object is not created
	bool OnCreate(CExporterPropertiesDlg *pPropDialog);

	// Check if ExportObject supports a given ExportObject instance as parent
	bool SupportsParentType(const CExportObject *pParent) const;

	// Export object
	bool Export(CExportProgressDlg *pProgressDlg, bool bForceAll);

private:
	CDDObject* BuildMetaDesc( void );

	static CPosePropertiesDlg* m_pEditDlg;
	static CDDObject* m_pDDMetaDesc;
};

DECLARE_EXPORT_OBJECT(CPoseAnimExportObject, "pose_animation", "Pose Animation", IDI_ICON_POSEANIM)

//

#endif // __NDS_LexiExporter_ExportObject_PoseAnim__