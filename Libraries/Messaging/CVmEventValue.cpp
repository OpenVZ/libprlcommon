/*
 * CVmEventValue.cpp
 *
 * Copyright (c) 1999-2017, Parallels International GmbH
 * Copyright (c) 2017-2019 Virtuozzo International GmbH. All rights reserved.
 *
 * This file is part of Virtuozzo SDK. Virtuozzo SDK is free
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
 * Our contact details: Virtuozzo International GmbH, Vordergasse 59, 8200
 * Schaffhausen, Switzerland.
 */

#include "CVmEventValue.h"


CVmEventValue::CVmEventValue()
{
	InitLists();
	setDefaults();
}

CVmEventValue::CVmEventValue(const CVmEventValue& rObject)
: CBaseNode()
{
	Copy(rObject);
}

CVmEventValue::CVmEventValue(const CVmEventValue* pObject)
{
	if (pObject)
	{
		Copy(*pObject);
	}
	else
	{
		InitLists();
		setDefaults();
	}
}

CVmEventValue& CVmEventValue::operator=(const CVmEventValue& rObject)
{
	Copy(rObject);
	return *this;
}

CVmEventValue::CVmEventValue(QFile* pFile)
{
	InitLists();
	loadFromFile(pFile);
}

CVmEventValue::~CVmEventValue()
{
	ClearLists();
}


QList<QString> CVmEventValue::getListItem() const
{
	return m_lstListItem;
}

void CVmEventValue::setListItem(QList<QString> value)
{
	m_lstListItem = value;
}

void CVmEventValue::setDefaults(QDomElement* RootElement)
{
	Q_UNUSED(RootElement);

	QStringList dyn_lists;
	if (RootElement)
		dyn_lists = RootElement->attribute("dyn_lists").split(" ");

	if ( ! RootElement || dyn_lists.contains("ListItem") )
		setListItem();

	static bool first_time = true;
	if ( first_time )
	{
		qRegisterMetaType< QList<QString> >("QList<QString>");
		first_time = false;
	}

}

QVariant CVmEventValue::getPropertyValue(QString path) const
{
	QVariant value;
	QString list_tag;
	bool bOk = false;
	Q_UNUSED(path);
	Q_UNUSED(value);
	Q_UNUSED(list_tag);
	Q_UNUSED(bOk);

	if (path == "itemId")
		value.setValue(getItemId());

	if (path == "ListItem.patch_stamp")
		value.setValue(getFieldPatchedValue("ListItem"));

	if (path == "ListItem")
		value.setValue(getListItem());

	return value;
}

bool CVmEventValue::setPropertyValue(QString path, QVariant value, bool* pbValueChanged)
{
	QString list_tag;
	bool bOk = false;
	Q_UNUSED(path);
	Q_UNUSED(value);
	Q_UNUSED(pbValueChanged);
	Q_UNUSED(list_tag);
	Q_UNUSED(bOk);

	if (path == "ListItem.patch_stamp")
	{
		markPatchedField("ListItem", value.toString());
		return true;
	}

	if (path == "ListItem")
	{
		if (pbValueChanged)
			*pbValueChanged = (getListItem() != value.value< QList<QString> >());
		setListItem(value.value< QList<QString> >());
		return true;
	}

	return false;
}

QString CVmEventValue::getListItem_id() const
{
	return getFullItemId().isEmpty() ? "ListItem" : getFullItemId() + ".ListItem";
}

int CVmEventValue::addListItem(QString path)
{
	int nNewItemId = -1;
	QString list_tag;
	bool bOk = false;
	Q_UNUSED(path);
	Q_UNUSED(nNewItemId);
	Q_UNUSED(list_tag);
	Q_UNUSED(bOk);

	return nNewItemId;
}

bool CVmEventValue::deleteListItem(QString path)
{
	QString list_tag;
	bool bOk = false;
	Q_UNUSED(path);
	Q_UNUSED(list_tag);
	Q_UNUSED(bOk);

	return false;
}

QDomElement CVmEventValue::getXml(QDomDocument* Document, bool no_save_option) const
{
	Q_UNUSED(no_save_option);
	int i = -1;
	Q_UNUSED(i);
	int nMaxItemId = -1;
	Q_UNUSED(nMaxItemId);
	int nElemIdx = 0;
	Q_UNUSED(nElemIdx);
	QDomElement element;
	QDomText text_element;
	QDomCDATASection cdata_element;
	QStringList dyn_lists;
	QSet<int > setItemIds;
	QMap<QString , int > mapMaxDynListIds = m_mapMaxDynListIds;

	QDomElement root_element = Document->createElement(m_qsExtRootTagName.isEmpty() ? QString("Value") : m_qsExtRootTagName);

	dyn_lists += "ListItem";
	for(i = 0; i < m_lstListItem.size(); ++i)
	{
		checkAndInsertExtDocElement(root_element, nElemIdx);
		element = Document->createElement("ListItem");
		text_element = Document->createTextNode( QString("%1").arg(m_lstListItem[i]) );
		root_element.appendChild(element);
		element.appendChild(text_element);
	}

	checkAndInsertExtDocElement(root_element, nElemIdx);

	root_element.setAttribute("dyn_lists", dyn_lists.join(" "));

	return root_element;
}

int CVmEventValue::readXml(QDomElement* RootElement, QString ext_tag_name, bool unite_with_loaded)
{
	QDomDocument temp_doc;
	QString tag_name;
	QString attribute;
	QDomText text_element;
	QDomCDATASection cdata_element;

	tag_name = RootElement->tagName();
	m_qsTagName = tag_name;
	m_qsExtRootTagName = ext_tag_name;
	if (eqName(tag_name, (ext_tag_name.isEmpty() ? QString("Value") : ext_tag_name), true))
	{
		m_qsErrorMessage = "Error in class 'CVmEventValue': wrong root element with tag name '" + tag_name + "'";
		return PRL_ERR_PARSE_VM_CONFIG;
	}

	bool bSupportDynList = RootElement->hasAttribute("dyn_lists");
	QStringList dyn_lists = RootElement->attribute("dyn_lists").split(" ");
	initMaxItemIds(dyn_lists);
	QDomElement element = RootElement->firstChildElement();


	m_qsErrorMessage.clear();
	m_lstWarningList.clear();
	ClearListsInReadXml(unite_with_loaded, dyn_lists, bSupportDynList);
	setDefaults(unite_with_loaded ? RootElement : 0);
	m_mapPatchedFields.clear();

	int ListItem_count = 0;

	int nElemIdx = 0;
	m_mapExtDoc.clear();
	m_doc = QDomDocument();
	while(!element.isNull())
	{
		tag_name = element.tagName();

		bool unused_tag = true;

		QDomElement* pElement = 0;
		Q_UNUSED(pElement);
		bool is_set = false;
		Q_UNUSED(is_set);

		if (eqName(tag_name, "ListItem"))
		{
			unused_tag = false;
			text_element = element.firstChild().toText();
			m_lstListItem += text_element.data();
			--ListItem_count;
		}
		if ( unused_tag )
			m_mapExtDoc.insert(nElemIdx, m_doc.importNode(element, true).toElement());
		nElemIdx++;
		element = element.nextSiblingElement();
	}

	if (ListItem_count >= 0)
	{
		m_lstWarningList += "Warning in class 'CVmEventValue': tag 'ListItem' is absent";
	}



	return 0;
}

void CVmEventValue::syncItemIds()
{
	int i = -1;
	Q_UNUSED(i);
	int nMaxItemId = -1;
	Q_UNUSED(nMaxItemId);
	QSet<int > setItemIds;

}

bool CVmEventValue::merge(CVmEventValue* pCur, CVmEventValue* pPrev, MergeOptions nOptions)
{
	int i = -1;
	Q_UNUSED(i);
	Q_UNUSED(pCur);
	Q_UNUSED(pPrev);
	Q_UNUSED(nOptions);

	m_qsErrorMessage = "Value";

	if (getListItem() != pCur->getListItem() && getListItem() == pPrev->getListItem())
		setListItem(pCur->getListItem());
	else if (getListItem() != pCur->getListItem() && getListItem() != pPrev->getListItem() && pCur->getListItem() != pPrev->getListItem())
	{
		m_qsErrorMessage += ".ListItem";
		return false;
	}

	return true;
}

void CVmEventValue::diff(const CVmEventValue* pOld, QStringList& lstDiffFullItemIds) const
{
	int i = -1;
	Q_UNUSED(i);
	Q_UNUSED(pOld);
	Q_UNUSED(lstDiffFullItemIds);

	if (getListItem() != pOld->getListItem())
		lstDiffFullItemIds += getListItem_id();

}

void CVmEventValue::Copy(const CVmEventValue& rObject)
{
	if (this == &rObject)
		return;

	int i = -1;
	Q_UNUSED(i);

	CopyBase(&rObject);

	setListItem(rObject.getListItem());

	CustomCopy(&rObject);

}

void CVmEventValue::InitLists()
{
	CustomInit();
}

void CVmEventValue::ClearLists()
{
}

void CVmEventValue::ClearListsInReadXml(bool unite_with_loaded, const QStringList& dyn_lists, bool bSupportDynList)
{
	Q_UNUSED(unite_with_loaded);
	Q_UNUSED(dyn_lists);
	Q_UNUSED(bSupportDynList);
	QDomElement element;
	int i = -1;
	Q_UNUSED(i);
}

