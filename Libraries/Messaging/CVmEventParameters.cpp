/*
 * CVmEventParameters.cpp
 *
 * Copyright (C) 1999-2016 Parallels IP Holdings GmbH
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
 * Our contact details: Parallels IP Holdings GmbH, Vordergasse 59, 8200
 * Schaffhausen, Switzerland.
 */

#include "CVmEventParameters.h"


CVmEventParameters::CVmEventParameters()
{
	InitLists();
	setDefaults();
}

CVmEventParameters::CVmEventParameters(const CVmEventParameters& rObject)
: CBaseNode()
{
	Copy(rObject);
}

CVmEventParameters::CVmEventParameters(const CVmEventParameters* pObject)
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

CVmEventParameters& CVmEventParameters::operator=(const CVmEventParameters& rObject)
{
	Copy(rObject);
	return *this;
}

CVmEventParameters::CVmEventParameters(QFile* pFile)
{
	InitLists();
	loadFromFile(pFile);
}

CVmEventParameters::~CVmEventParameters()
{
	ClearLists();
}


void CVmEventParameters::setDefaults(QDomElement* RootElement)
{
	Q_UNUSED(RootElement);

	QStringList dyn_lists;
	if (RootElement)
		dyn_lists = RootElement->attribute("dyn_lists").split(" ");

}

QVariant CVmEventParameters::getPropertyValue(QString path) const
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

	list_tag = "EventParameter[";
	if (path.startsWith(list_tag))
	{
		int idx = path.indexOf("].");
		if (idx == -1) return value;
		QString qsItemId = path.mid(list_tag.size(), idx - list_tag.size());
		int nItemId = qsItemId.toInt(&bOk);
		if ( ! bOk ) return value;
		for(int i = 0; i < m_lstEventParameter.size(); ++i)
			if (m_lstEventParameter[i] && nItemId == m_lstEventParameter[i]->getItemId())
				return m_lstEventParameter[i]->getPropertyValue(path.mid(idx + 2));
	}
	else if (path.startsWith("EventParameter.maxItemId"))
	{
		int nMaxId = -1;
		for(int i = 0; i < m_lstEventParameter.size(); ++i)
			if (m_lstEventParameter[i] && nMaxId < m_lstEventParameter[i]->getItemId())
				nMaxId = m_lstEventParameter[i]->getItemId();
		value.setValue(nMaxId);
	}
	else if (path.startsWith("EventParameter.listItemIds"))
	{
		QList<int > lstIds;
		for(int i = 0; i < m_lstEventParameter.size(); ++i)
			if (m_lstEventParameter[i]) lstIds += m_lstEventParameter[i]->getItemId();
		value.setValue(lstIds);
	}

	return value;
}

bool CVmEventParameters::setPropertyValue(QString path, QVariant value, bool* pbValueChanged)
{
	QString list_tag;
	bool bOk = false;
	Q_UNUSED(path);
	Q_UNUSED(value);
	Q_UNUSED(pbValueChanged);
	Q_UNUSED(list_tag);
	Q_UNUSED(bOk);

	list_tag = "EventParameter[";
	if (path.startsWith(list_tag))
	{
		int idx = path.indexOf("].");
		if (idx == -1) return false;
		QString qsItemId = path.mid(list_tag.size(), idx - list_tag.size());
		int nItemId = qsItemId.toInt(&bOk);
		if ( ! bOk ) return false;
		for(int i = 0; i < m_lstEventParameter.size(); ++i)
			if (m_lstEventParameter[i] && nItemId == m_lstEventParameter[i]->getItemId())
				return m_lstEventParameter[i]->setPropertyValue(path.mid(idx + 2), value, pbValueChanged);
	}

	return false;
}

int CVmEventParameters::addListItem(QString path)
{
	int nNewItemId = -1;
	QString list_tag;
	bool bOk = false;
	Q_UNUSED(path);
	Q_UNUSED(nNewItemId);
	Q_UNUSED(list_tag);
	Q_UNUSED(bOk);

	if (path == "EventParameter")
	{
		m_lstEventParameter += new CVmEventParameter;
		nNewItemId = getMaxItemId<CVmEventParameter>(m_lstEventParameter, "EventParameter");
		m_lstEventParameter.last()->setItemId(nNewItemId);
		syncMaxItemId(nNewItemId + 1, "EventParameter");
	}

	list_tag = "EventParameter[";
	if (path.startsWith(list_tag))
	{
		int idx = path.indexOf("].");
		if (idx == -1) return -1;
		QString qsItemId = path.mid(list_tag.size(), idx - list_tag.size());
		int nItemId = qsItemId.toInt(&bOk);
		if ( ! bOk ) return -1;
		for(int i = 0; i < m_lstEventParameter.size(); ++i)
			if (m_lstEventParameter[i] && nItemId == m_lstEventParameter[i]->getItemId())
				return m_lstEventParameter[i]->addListItem(path.mid(idx + 2));
	}

	return nNewItemId;
}

bool CVmEventParameters::deleteListItem(QString path)
{
	QString list_tag;
	bool bOk = false;
	Q_UNUSED(path);
	Q_UNUSED(list_tag);
	Q_UNUSED(bOk);

	list_tag = "EventParameter[";
	if (path.startsWith(list_tag))
	{
		bool bDel = false;
		int idx = path.indexOf("].");
		if (idx == -1 && (idx = path.indexOf("]")) != -1) bDel = true;
		if (idx == -1) return false;
		QString qsItemId = path.mid(list_tag.size(), idx - list_tag.size());
		int nItemId = qsItemId.toInt(&bOk);
		if ( ! bOk ) return false;
		for(int i = 0; i < m_lstEventParameter.size(); ++i)
			if (m_lstEventParameter[i] && nItemId == m_lstEventParameter[i]->getItemId())
			{
				if ( ! bDel )
					return m_lstEventParameter[i]->deleteListItem(path.mid(idx + 2));
				delete m_lstEventParameter.takeAt(i);
				return true;
			}
	}

	return false;
}

QDomElement CVmEventParameters::getXml(QDomDocument* Document, bool no_save_option) const
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

	QDomElement root_element = Document->createElement(m_qsExtRootTagName.isEmpty() ? QString("EventParameters") : m_qsExtRootTagName);

	dyn_lists += "EventParameter";
	nMaxItemId = getMaxItemId<CVmEventParameter>(m_lstEventParameter, "EventParameter", mapMaxDynListIds);
	setItemIds.clear();
	for(i = 0; i < m_lstEventParameter.size() && (true); ++i)
	{
		checkAndInsertExtDocElement(root_element, nElemIdx);
		const CVmEventParameter* object = m_lstEventParameter[i];
		if ( ! object ) continue;
		int nItemId = object->getItemId();
		do {
			nItemId = nItemId == -1 ? nMaxItemId++ : nItemId;
			if ( ! setItemIds.contains(nItemId) ) break;
			nItemId = -1;
		} while(1);
		setItemIds.insert(nItemId);
		element = object->getXml(Document, no_save_option);
		element.setAttribute("id", QString::number(nItemId));
		root_element.appendChild(element);
	}
	dyn_lists += QString::number(syncMaxItemId(nMaxItemId, "EventParameter", mapMaxDynListIds));

	checkAndInsertExtDocElement(root_element, nElemIdx);

	root_element.setAttribute("dyn_lists", dyn_lists.join(" "));

	return root_element;
}

int CVmEventParameters::readXml(QDomElement* RootElement, QString ext_tag_name, bool unite_with_loaded)
{
	QDomDocument temp_doc;
	QString tag_name;
	QString attribute;
	QDomText text_element;
	QDomCDATASection cdata_element;

	tag_name = RootElement->tagName();
	m_qsTagName = tag_name;
	m_qsExtRootTagName = ext_tag_name;
	if (tag_name != (ext_tag_name.isEmpty() ? QString("EventParameters") : ext_tag_name))
	{
		m_qsErrorMessage = "Error in class 'CVmEventParameters': wrong root element with tag name '" + tag_name + "'";
		return PRL_ERR_PARSE_VM_CONFIG;
	}

	bool bSupportDynList = RootElement->hasAttribute("dyn_lists");
	QStringList dyn_lists = RootElement->attribute("dyn_lists").split(" ");
	initMaxItemIds(dyn_lists);
	QDomElement element = RootElement->firstChildElement();

	int nMaxItemId_EventParameter = getMaxItemId2(element, "EventParameter");
	QMap<int , QString > mapEventParameter;
	if (unite_with_loaded)
		fillItemIdMap<CVmEventParameter>(m_lstEventParameter, mapEventParameter);

	m_qsErrorMessage.clear();
	m_lstWarningList.clear();
	ClearListsInReadXml(unite_with_loaded, dyn_lists, bSupportDynList);
	setDefaults(unite_with_loaded ? RootElement : 0);
	m_mapPatchedFields.clear();

	int EventParameter_count = 0;

	int nElemIdx = 0;
	m_mapExtDoc.clear();
	m_doc = QDomDocument();
	while(!element.isNull())
	{
		tag_name = element.tagName();

		bool unused_tag = true;

		if (tag_name == "EventParameter")
		{
			unused_tag = false;
			int nItemId = element.attribute("id", "-1").toInt();
			nItemId = nItemId == -1 ? nMaxItemId_EventParameter++ : nItemId;
			CVmEventParameter* object = new CVmEventParameter();
			object->setItemId(nItemId);
			object->makeFullItemId(getFullItemId(), "EventParameter", object->getItemId());
			if (unite_with_loaded && mapEventParameter.contains(nItemId))
				object->fromString(mapEventParameter.value(nItemId));
			if (object->readXml(&element, tag_name, unite_with_loaded))
			{
				m_qsErrorMessage = object->GetErrorMessage();
				delete object;
				return PRL_ERR_PARSE_VM_CONFIG;
			}
			m_lstEventParameter += object;
			--EventParameter_count;
			m_lstWarningList += object->GetWarningList();
		}

		if ( unused_tag )
			m_mapExtDoc.insert(nElemIdx, m_doc.importNode(element, true).toElement());
		nElemIdx++;
		element = element.nextSiblingElement();
	}

	if (EventParameter_count >= 0)
	{
		m_lstWarningList += "Warning in class 'CVmEventParameters': tag 'EventParameter' is absent";
	}

	syncMaxItemId(nMaxItemId_EventParameter, "EventParameter");


	return 0;
}

void CVmEventParameters::syncItemIds()
{
	int i = -1;
	Q_UNUSED(i);
	int nMaxItemId = -1;
	Q_UNUSED(nMaxItemId);
	QSet<int > setItemIds;

	nMaxItemId = getMaxItemId<CVmEventParameter>(m_lstEventParameter, "EventParameter");
	setItemIds.clear();
	for(i = 0; i < m_lstEventParameter.size(); ++i)
	{
		CVmEventParameter* object = m_lstEventParameter[i];
		if ( ! object ) continue;
		int nItemId = object->getItemId();
		do {
			nItemId = nItemId == -1 ? nMaxItemId++ : nItemId;
			if ( ! setItemIds.contains(nItemId) ) break;
			nItemId = -1;
		} while(1);
		setItemIds.insert(nItemId);
		object->setItemId(nItemId);
		object->makeFullItemId(getFullItemId(), "EventParameter", object->getItemId());
		object->syncItemIds();
	}
	syncMaxItemId(nMaxItemId, "EventParameter");

}

bool CVmEventParameters::merge(CVmEventParameters* pCur, CVmEventParameters* pPrev, MergeOptions nOptions)
{
	int i = -1;
	Q_UNUSED(i);
	Q_UNUSED(pCur);
	Q_UNUSED(pPrev);
	Q_UNUSED(nOptions);

	m_qsErrorMessage = "EventParameters";

	if ( ! mergeList<CVmEventParameter>(m_lstEventParameter, pCur->m_lstEventParameter, pPrev->m_lstEventParameter, "EventParameter", nOptions) )
		return false;

	return true;
}

void CVmEventParameters::diff(const CVmEventParameters* pOld, QStringList& lstDiffFullItemIds) const
{
	int i = -1;
	Q_UNUSED(i);
	Q_UNUSED(pOld);
	Q_UNUSED(lstDiffFullItemIds);

	diffList<CVmEventParameter>(m_lstEventParameter, pOld->m_lstEventParameter, lstDiffFullItemIds);

}

void CVmEventParameters::Copy(const CVmEventParameters& rObject)
{
	if (this == &rObject)
		return;

	int i = -1;
	Q_UNUSED(i);

	CopyBase(&rObject);


	ClearList<CVmEventParameter >(m_lstEventParameter);
	for(i = 0; i < rObject.m_lstEventParameter.size(); i++)
		m_lstEventParameter += new CVmEventParameter(rObject.m_lstEventParameter[i]);

	CustomCopy(&rObject);

}

void CVmEventParameters::InitLists()
{
	CustomInit();
}

void CVmEventParameters::ClearLists()
{
	ClearList<CVmEventParameter>(m_lstEventParameter);
}

void CVmEventParameters::ClearListsInReadXml(bool unite_with_loaded, const QStringList& dyn_lists, bool bSupportDynList)
{
	Q_UNUSED(unite_with_loaded);
	Q_UNUSED(dyn_lists);
	Q_UNUSED(bSupportDynList);
	QDomElement element;
	int i = -1;
	Q_UNUSED(i);
	if ( ! unite_with_loaded || ! bSupportDynList || dyn_lists.contains("EventParameter") )
		ClearList<CVmEventParameter>(m_lstEventParameter);
}

