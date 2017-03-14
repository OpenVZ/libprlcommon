/*
 * CVmEventValue.h: Definition of the class CVmEventValue.
 *
 * Copyright (c) 1999-2017, Parallels International GmbH
 *
 * This file is part of Parallels SDK. Parallels SDK is free
 * software; you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License,
 * or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/> or write to Free Software Foundation,
 * 51 Franklin Street, Fifth Floor Boston, MA 02110, USA.
 *
 * Our contact details: Parallels International GmbH, Vordergasse 59, 8200
 * Schaffhausen, Switzerland.
 */

#ifndef CVMEVENTVALUE_H
#define CVMEVENTVALUE_H


#include "../PrlObjects/CBaseNode.h"


class CVmEventValue : public CBaseNode
{

public:

	CVmEventValue();
	CVmEventValue(const CVmEventValue& rObject);
	CVmEventValue(const CVmEventValue* pObject);
	CVmEventValue(QFile* pFile);
	CVmEventValue& operator=(const CVmEventValue& rObject);
	virtual ~CVmEventValue();




	QList<QString> getListItem() const;
	void setListItem(QList<QString> value = QList<QString>());

	virtual void setDefaults(QDomElement* RootElement = 0);

	virtual QVariant getPropertyValue(QString path) const;
	virtual bool setPropertyValue(QString path, QVariant value, bool* pbValueChanged = 0);

	QString getListItem_id() const;

	int addListItem(QString path);
	bool deleteListItem(QString path);

	virtual QDomElement getXml(QDomDocument* Document, bool no_save_option = false) const;
	virtual int readXml(QDomElement* RootElement, QString ext_tag_name = QString(), bool unite_with_loaded = false);
	virtual void syncItemIds();

	bool merge(CVmEventValue* pCur, CVmEventValue* pPrev, MergeOptions nOptions);

	void diff(const CVmEventValue* pOld, QStringList& lstDiffFullItemIds) const;

	virtual void InitLists();
	virtual void ClearLists();
	virtual void ClearListsInReadXml(bool unite_with_loaded = false, const QStringList& dyn_lists = QStringList(), bool bSupportDynList = true);

protected:

	QList<QString>		m_lstListItem;

	void Copy(const CVmEventValue& rObject);

};

#ifndef DM_212734958
#define DM_212734958
Q_DECLARE_METATYPE(QList<QString>)
#endif


#endif
