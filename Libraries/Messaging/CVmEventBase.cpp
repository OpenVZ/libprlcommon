/*
 * CVmEventBase.cpp
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

#include "CVmEventBase.h"


CVmEventBase::CVmEventBase()
{
	InitLists();
	setDefaults();
}

CVmEventBase::CVmEventBase(const CVmEventBase& rObject)
: CPrlDataSerializer(), CBaseNode()
{
	Copy(rObject);
}

CVmEventBase::CVmEventBase(const CVmEventBase* pObject)
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

CVmEventBase& CVmEventBase::operator=(const CVmEventBase& rObject)
{
	Copy(rObject);
	return *this;
}

CVmEventBase::CVmEventBase(QFile* pFile)
{
	InitLists();
	loadFromFile(pFile);
}

CVmEventBase::~CVmEventBase()
{
	ClearLists();
}


CVmEventParameters* CVmEventBase::getEventParameters() const
{
	if (!m_lstBaseEventParameters.isEmpty())
	{
		return m_lstBaseEventParameters[0];
	}
	return 0;
}

void CVmEventBase::setEventParameters(CVmEventParameters* pBaseEventParameters)
{
	ClearList<CVmEventParameters>(m_lstBaseEventParameters);
	if (pBaseEventParameters)
	{
		m_lstBaseEventParameters += pBaseEventParameters;
	}
}

PRL_EVENT_TYPE CVmEventBase::getEventType() const
{
	return m_ctEventType;
}

void CVmEventBase::setEventType(PRL_EVENT_TYPE value)
{
	m_ctEventType = value;
}

PVE::VmEventLevel CVmEventBase::getEventLevel() const
{
	return m_ctEventLevel;
}

void CVmEventBase::setEventLevel(PVE::VmEventLevel value)
{
	m_ctEventLevel = value;
}

PRL_RESULT CVmEventBase::getEventCode() const
{
	return m_ctEventCode;
}

void CVmEventBase::setEventCode(PRL_RESULT value)
{
	m_ctEventCode = value;
}

PVE::VmEventRespOption CVmEventBase::getRespRequired() const
{
	return m_ctEventNeedResponse;
}

void CVmEventBase::setRespRequired(PVE::VmEventRespOption value)
{
	m_ctEventNeedResponse = value;
}

PRL_EVENT_ISSUER_TYPE CVmEventBase::getEventIssuerType() const
{
	return m_ctEventIssuerType;
}

void CVmEventBase::setEventIssuerType(PRL_EVENT_ISSUER_TYPE value)
{
	m_ctEventIssuerType = value;
}

QString CVmEventBase::getEventIssuerId() const
{
	return m_qsEventIssuerId;
}

void CVmEventBase::setEventIssuerId(QString value)
{
	m_qsEventIssuerId = value;
}

QString CVmEventBase::getEventSource() const
{
	return m_qsEventSource;
}

void CVmEventBase::setEventSource(QString value)
{
	m_qsEventSource = value;
}

QString CVmEventBase::getInitRequestId() const
{
	return m_qsEventInitialRequestId;
}

void CVmEventBase::setInitRequestId(QString value)
{
	m_qsEventInitialRequestId = value;
}

qulonglong CVmEventBase::getEventId() const
{
	return m_ullEventId;
}

void CVmEventBase::setEventId(qulonglong value)
{
	m_ullEventId = value;
}

void CVmEventBase::setDefaults(QDomElement* RootElement)
{
	Q_UNUSED(RootElement);

	QStringList dyn_lists;
	if (RootElement)
		dyn_lists = RootElement->attribute("dyn_lists").split(" ");

	if ( ! RootElement || ! RootElement->firstChildElement("EventType").isNull() )
		setEventType();
	if ( ! RootElement || ! RootElement->firstChildElement("EventLevel").isNull() )
		setEventLevel();
	if ( ! RootElement || ! RootElement->firstChildElement("EventCode").isNull() )
		setEventCode();
	if ( ! RootElement || ! RootElement->firstChildElement("EventNeedResponse").isNull() )
		setRespRequired();
	if ( ! RootElement || ! RootElement->firstChildElement("EventIssuerType").isNull() )
		setEventIssuerType();
	if ( ! RootElement || ! RootElement->firstChildElement("EventIssuerId").isNull() )
		setEventIssuerId(Uuid::createUuid().toString());
	if ( ! RootElement || ! RootElement->firstChildElement("EventSource").isNull() )
		setEventSource();
	if ( ! RootElement || ! RootElement->firstChildElement("EventInitialRequestId").isNull() )
		setInitRequestId(Uuid::createUuid().toString());
	if ( ! RootElement || ! RootElement->firstChildElement("EventId").isNull() )
		setEventId();

	static bool first_time = true;
	if ( first_time )
	{
		qRegisterMetaType< PRL_EVENT_TYPE >("PRL_EVENT_TYPE");
		qRegisterMetaType< PVE::VmEventLevel >("PVE::VmEventLevel");
		qRegisterMetaType< PRL_RESULT >("PRL_RESULT");
		qRegisterMetaType< PVE::VmEventRespOption >("PVE::VmEventRespOption");
		qRegisterMetaType< PRL_EVENT_ISSUER_TYPE >("PRL_EVENT_ISSUER_TYPE");
		first_time = false;
	}

}

QVariant CVmEventBase::getPropertyValue(QString path) const
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

	if (path == "EventType.patch_stamp")
		value.setValue(getFieldPatchedValue("EventType"));

	if (path == "EventType")
		value.setValue((qlonglong )getEventType());

	if (path == "EventLevel.patch_stamp")
		value.setValue(getFieldPatchedValue("EventLevel"));

	if (path == "EventLevel")
		value.setValue((qlonglong )getEventLevel());

	if (path == "EventCode.patch_stamp")
		value.setValue(getFieldPatchedValue("EventCode"));

	if (path == "EventCode")
		value.setValue((qlonglong )getEventCode());

	if (path == "EventNeedResponse.patch_stamp")
		value.setValue(getFieldPatchedValue("EventNeedResponse"));

	if (path == "EventNeedResponse")
		value.setValue((qlonglong )getRespRequired());

	if (path == "EventIssuerType.patch_stamp")
		value.setValue(getFieldPatchedValue("EventIssuerType"));

	if (path == "EventIssuerType")
		value.setValue((qlonglong )getEventIssuerType());

	if (path == "EventIssuerId.patch_stamp")
		value.setValue(getFieldPatchedValue("EventIssuerId"));

	if (path == "EventIssuerId")
		value.setValue(getEventIssuerId());

	if (path == "EventSource.patch_stamp")
		value.setValue(getFieldPatchedValue("EventSource"));

	if (path == "EventSource")
		value.setValue(getEventSource());

	if (path == "EventInitialRequestId.patch_stamp")
		value.setValue(getFieldPatchedValue("EventInitialRequestId"));

	if (path == "EventInitialRequestId")
		value.setValue(getInitRequestId());

	if (path == "EventId.patch_stamp")
		value.setValue(getFieldPatchedValue("EventId"));

	if (path == "EventId")
		value.setValue(getEventId());

	list_tag = "EventParameters.";
	if (path.startsWith(list_tag) && ! m_lstBaseEventParameters.isEmpty() && m_lstBaseEventParameters.first())
		return m_lstBaseEventParameters.first()->getPropertyValue(path.mid(list_tag.size()));

	return value;
}

bool CVmEventBase::setPropertyValue(QString path, QVariant value, bool* pbValueChanged)
{
	QString list_tag;
	bool bOk = false;
	Q_UNUSED(path);
	Q_UNUSED(value);
	Q_UNUSED(pbValueChanged);
	Q_UNUSED(list_tag);
	Q_UNUSED(bOk);

	if (path == "EventType.patch_stamp")
	{
		markPatchedField("EventType", value.toString());
		return true;
	}

	if (path == "EventType")
	{
		if (pbValueChanged)
			*pbValueChanged = (getEventType() != (PRL_EVENT_TYPE )value.value< qlonglong >());
		setEventType((PRL_EVENT_TYPE )value.value< qlonglong >());
		return true;
	}

	if (path == "EventLevel.patch_stamp")
	{
		markPatchedField("EventLevel", value.toString());
		return true;
	}

	if (path == "EventLevel")
	{
		if (pbValueChanged)
			*pbValueChanged = (getEventLevel() != (PVE::VmEventLevel )value.value< qlonglong >());
		setEventLevel((PVE::VmEventLevel )value.value< qlonglong >());
		return true;
	}

	if (path == "EventCode.patch_stamp")
	{
		markPatchedField("EventCode", value.toString());
		return true;
	}

	if (path == "EventCode")
	{
		if (pbValueChanged)
			*pbValueChanged = (getEventCode() != (PRL_RESULT )value.value< qlonglong >());
		setEventCode((PRL_RESULT )value.value< qlonglong >());
		return true;
	}

	if (path == "EventNeedResponse.patch_stamp")
	{
		markPatchedField("EventNeedResponse", value.toString());
		return true;
	}

	if (path == "EventNeedResponse")
	{
		if (pbValueChanged)
			*pbValueChanged = (getRespRequired() != (PVE::VmEventRespOption )value.value< qlonglong >());
		setRespRequired((PVE::VmEventRespOption )value.value< qlonglong >());
		return true;
	}

	if (path == "EventIssuerType.patch_stamp")
	{
		markPatchedField("EventIssuerType", value.toString());
		return true;
	}

	if (path == "EventIssuerType")
	{
		if (pbValueChanged)
			*pbValueChanged = (getEventIssuerType() != (PRL_EVENT_ISSUER_TYPE )value.value< qlonglong >());
		setEventIssuerType((PRL_EVENT_ISSUER_TYPE )value.value< qlonglong >());
		return true;
	}

	if (path == "EventIssuerId.patch_stamp")
	{
		markPatchedField("EventIssuerId", value.toString());
		return true;
	}

	if (path == "EventIssuerId")
	{
		if (pbValueChanged)
			*pbValueChanged = (getEventIssuerId() != value.value< QString >());
		setEventIssuerId(value.value< QString >());
		return true;
	}

	if (path == "EventSource.patch_stamp")
	{
		markPatchedField("EventSource", value.toString());
		return true;
	}

	if (path == "EventSource")
	{
		if (pbValueChanged)
			*pbValueChanged = (getEventSource() != value.value< QString >());
		setEventSource(value.value< QString >());
		return true;
	}

	if (path == "EventInitialRequestId.patch_stamp")
	{
		markPatchedField("EventInitialRequestId", value.toString());
		return true;
	}

	if (path == "EventInitialRequestId")
	{
		if (pbValueChanged)
			*pbValueChanged = (getInitRequestId() != value.value< QString >());
		setInitRequestId(value.value< QString >());
		return true;
	}

	if (path == "EventId.patch_stamp")
	{
		markPatchedField("EventId", value.toString());
		return true;
	}

	if (path == "EventId")
	{
		if (pbValueChanged)
			*pbValueChanged = (getEventId() != value.value< qulonglong >());
		setEventId(value.value< qulonglong >());
		return true;
	}

	list_tag = "EventParameters.";
	if (path.startsWith(list_tag) && ! m_lstBaseEventParameters.isEmpty() && m_lstBaseEventParameters.first())
		return m_lstBaseEventParameters.first()->setPropertyValue(path.mid(list_tag.size()), value, pbValueChanged);

	return false;
}

QString CVmEventBase::getEventType_id() const
{
	return getFullItemId().isEmpty() ? "EventType" : getFullItemId() + ".EventType";
}
QString CVmEventBase::getEventLevel_id() const
{
	return getFullItemId().isEmpty() ? "EventLevel" : getFullItemId() + ".EventLevel";
}
QString CVmEventBase::getEventCode_id() const
{
	return getFullItemId().isEmpty() ? "EventCode" : getFullItemId() + ".EventCode";
}
QString CVmEventBase::getRespRequired_id() const
{
	return getFullItemId().isEmpty() ? "EventNeedResponse" : getFullItemId() + ".EventNeedResponse";
}
QString CVmEventBase::getEventIssuerType_id() const
{
	return getFullItemId().isEmpty() ? "EventIssuerType" : getFullItemId() + ".EventIssuerType";
}
QString CVmEventBase::getEventIssuerId_id() const
{
	return getFullItemId().isEmpty() ? "EventIssuerId" : getFullItemId() + ".EventIssuerId";
}
QString CVmEventBase::getEventSource_id() const
{
	return getFullItemId().isEmpty() ? "EventSource" : getFullItemId() + ".EventSource";
}
QString CVmEventBase::getInitRequestId_id() const
{
	return getFullItemId().isEmpty() ? "EventInitialRequestId" : getFullItemId() + ".EventInitialRequestId";
}
QString CVmEventBase::getEventId_id() const
{
	return getFullItemId().isEmpty() ? "EventId" : getFullItemId() + ".EventId";
}

int CVmEventBase::addListItem(QString path)
{
	int nNewItemId = -1;
	QString list_tag;
	bool bOk = false;
	Q_UNUSED(path);
	Q_UNUSED(nNewItemId);
	Q_UNUSED(list_tag);
	Q_UNUSED(bOk);

	list_tag = "EventParameters.";
	if (path.startsWith(list_tag) && ! m_lstBaseEventParameters.isEmpty() && m_lstBaseEventParameters.first())
		return m_lstBaseEventParameters.first()->addListItem(path.mid(list_tag.size()));

	return nNewItemId;
}

bool CVmEventBase::deleteListItem(QString path)
{
	QString list_tag;
	bool bOk = false;
	Q_UNUSED(path);
	Q_UNUSED(list_tag);
	Q_UNUSED(bOk);

	list_tag = "EventParameters.";
	if (path.startsWith(list_tag) && ! m_lstBaseEventParameters.isEmpty() && m_lstBaseEventParameters.first())
		return m_lstBaseEventParameters.first()->deleteListItem(path.mid(list_tag.size()));

	return false;
}

QDomElement CVmEventBase::getXml(QDomDocument* Document, bool no_save_option) const
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

	QDomElement root_element = Document->createElement(m_qsExtRootTagName.isEmpty() ? getLegacyProductTag("VirtuozzoEvent") : m_qsExtRootTagName);

	checkAndInsertExtDocElement(root_element, nElemIdx);
	element = Document->createElement("EventType");
	text_element = Document->createTextNode( QString("%1").arg(getEventType()) );
	root_element.appendChild(element);
	element.appendChild(text_element);

	checkAndInsertExtDocElement(root_element, nElemIdx);
	element = Document->createElement("EventLevel");
	text_element = Document->createTextNode( QString("%1").arg(getEventLevel()) );
	root_element.appendChild(element);
	element.appendChild(text_element);

	checkAndInsertExtDocElement(root_element, nElemIdx);
	element = Document->createElement("EventCode");
	text_element = Document->createTextNode( QString("%1").arg(getEventCode()) );
	root_element.appendChild(element);
	element.appendChild(text_element);

	checkAndInsertExtDocElement(root_element, nElemIdx);
	element = Document->createElement("EventNeedResponse");
	text_element = Document->createTextNode( QString("%1").arg(getRespRequired()) );
	root_element.appendChild(element);
	element.appendChild(text_element);

	checkAndInsertExtDocElement(root_element, nElemIdx);
	element = Document->createElement("EventIssuerType");
	text_element = Document->createTextNode( QString("%1").arg(getEventIssuerType()) );
	root_element.appendChild(element);
	element.appendChild(text_element);

	checkAndInsertExtDocElement(root_element, nElemIdx);
	element = Document->createElement("EventIssuerId");
	text_element = Document->createTextNode( QString("%1").arg(getEventIssuerId()) );
	root_element.appendChild(element);
	element.appendChild(text_element);

	checkAndInsertExtDocElement(root_element, nElemIdx);
	element = Document->createElement("EventSource");
	text_element = Document->createTextNode( QString("%1").arg(getEventSource()) );
	root_element.appendChild(element);
	element.appendChild(text_element);

	checkAndInsertExtDocElement(root_element, nElemIdx);
	element = Document->createElement("EventInitialRequestId");
	text_element = Document->createTextNode( QString("%1").arg(getInitRequestId()) );
	root_element.appendChild(element);
	element.appendChild(text_element);

	for(i = 0; i < m_lstBaseEventParameters.size() && (true); ++i)
	{
		checkAndInsertExtDocElement(root_element, nElemIdx);
		const CVmEventParameters* object = m_lstBaseEventParameters[i];
		if ( ! object ) continue;
		element = object->getXml(Document, no_save_option);
		root_element.appendChild(element);
	}

	checkAndInsertExtDocElement(root_element, nElemIdx);
	element = Document->createElement("EventId");
	text_element = Document->createTextNode( QString("%1").arg(getEventId()) );
	root_element.appendChild(element);
	element.appendChild(text_element);

	checkAndInsertExtDocElement(root_element, nElemIdx);

	root_element.setAttribute("dyn_lists", dyn_lists.join(" "));

	return root_element;
}

int CVmEventBase::readXml(QDomElement* RootElement, QString ext_tag_name, bool unite_with_loaded)
{
	QDomDocument temp_doc;
	QString tag_name;
	QString attribute;
	QDomText text_element;
	QDomCDATASection cdata_element;

	tag_name = RootElement->tagName();
	m_qsTagName = tag_name;
	m_qsExtRootTagName = ext_tag_name;
	if (!eqName(tag_name, (ext_tag_name.isEmpty() ? getLegacyProductTag("VirtuozzoEvent") : ext_tag_name), true))
	{
		m_qsErrorMessage = "Error in class 'CVmEventBase': wrong root element with tag name '" + tag_name + "'";
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

	int BaseEventParameters_count = 1;
	int EventType_count = 1;
	int EventLevel_count = 1;
	int EventCode_count = 1;
	int EventNeedResponse_count = 1;
	int EventIssuerType_count = 1;
	int EventIssuerId_count = 1;
	int EventSource_count = 1;
	int EventInitialRequestId_count = 1;
	int EventId_count = 0;

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

		pElement = 0;
		is_set = false;
		attribute = "EventType";
		if (RootElement->hasAttribute(attribute))
		{
			pElement = RootElement;
		}
		else if (element.hasAttribute(attribute))
		{
			pElement = &element;
		}

		if (pElement && (EventType_count - 1) == 0)
		{
			text_element = temp_doc.createTextNode( pElement->attribute(attribute) );
			is_set = true;
		}
		else if (eqName(tag_name, "EventType"))
		{
			is_set = true;
			text_element = element.firstChild().toText();
		}

		if (is_set)
		{
			unused_tag = false;
			setEventType((PRL_EVENT_TYPE )text_element.data().toLongLong());
			--EventType_count;
		}


		pElement = 0;
		is_set = false;
		attribute = "EventLevel";
		if (RootElement->hasAttribute(attribute))
		{
			pElement = RootElement;
		}
		else if (element.hasAttribute(attribute))
		{
			pElement = &element;
		}

		if (pElement && (EventLevel_count - 1) == 0)
		{
			text_element = temp_doc.createTextNode( pElement->attribute(attribute) );
			is_set = true;
		}
		else if (eqName(tag_name, "EventLevel"))
		{
			is_set = true;
			text_element = element.firstChild().toText();
		}

		if (is_set)
		{
			unused_tag = false;
			setEventLevel((PVE::VmEventLevel )text_element.data().toLongLong());
			--EventLevel_count;
		}


		pElement = 0;
		is_set = false;
		attribute = "EventCode";
		if (RootElement->hasAttribute(attribute))
		{
			pElement = RootElement;
		}
		else if (element.hasAttribute(attribute))
		{
			pElement = &element;
		}

		if (pElement && (EventCode_count - 1) == 0)
		{
			text_element = temp_doc.createTextNode( pElement->attribute(attribute) );
			is_set = true;
		}
		else if (eqName(tag_name, "EventCode"))
		{
			is_set = true;
			text_element = element.firstChild().toText();
		}

		if (is_set)
		{
			unused_tag = false;
			setEventCode((PRL_RESULT )text_element.data().toLongLong());
			--EventCode_count;
		}


		pElement = 0;
		is_set = false;
		attribute = "EventNeedResponse";
		if (RootElement->hasAttribute(attribute))
		{
			pElement = RootElement;
		}
		else if (element.hasAttribute(attribute))
		{
			pElement = &element;
		}

		if (pElement && (EventNeedResponse_count - 1) == 0)
		{
			text_element = temp_doc.createTextNode( pElement->attribute(attribute) );
			is_set = true;
		}
		else if (eqName(tag_name, "EventNeedResponse"))
		{
			is_set = true;
			text_element = element.firstChild().toText();
		}

		if (is_set)
		{
			unused_tag = false;
			setRespRequired((PVE::VmEventRespOption )text_element.data().toLongLong());
			--EventNeedResponse_count;
		}


		pElement = 0;
		is_set = false;
		attribute = "EventIssuerType";
		if (RootElement->hasAttribute(attribute))
		{
			pElement = RootElement;
		}
		else if (element.hasAttribute(attribute))
		{
			pElement = &element;
		}

		if (pElement && (EventIssuerType_count - 1) == 0)
		{
			text_element = temp_doc.createTextNode( pElement->attribute(attribute) );
			is_set = true;
		}
		else if (eqName(tag_name, "EventIssuerType"))
		{
			is_set = true;
			text_element = element.firstChild().toText();
		}

		if (is_set)
		{
			unused_tag = false;
			setEventIssuerType((PRL_EVENT_ISSUER_TYPE )text_element.data().toLongLong());
			--EventIssuerType_count;
		}


		pElement = 0;
		is_set = false;
		attribute = "EventIssuerId";
		if (RootElement->hasAttribute(attribute))
		{
			pElement = RootElement;
		}
		else if (element.hasAttribute(attribute))
		{
			pElement = &element;
		}

		if (pElement && (EventIssuerId_count - 1) == 0)
		{
			text_element = temp_doc.createTextNode( pElement->attribute(attribute) );
			is_set = true;
		}
		else if (eqName(tag_name, "EventIssuerId"))
		{
			is_set = true;
			text_element = element.firstChild().toText();
		}

		if (is_set)
		{
			unused_tag = false;
			setEventIssuerId(text_element.data());
			--EventIssuerId_count;
		}


		pElement = 0;
		is_set = false;
		attribute = "EventSource";
		if (RootElement->hasAttribute(attribute))
		{
			pElement = RootElement;
		}
		else if (element.hasAttribute(attribute))
		{
			pElement = &element;
		}

		if (pElement && (EventSource_count - 1) == 0)
		{
			text_element = temp_doc.createTextNode( pElement->attribute(attribute) );
			is_set = true;
		}
		else if (eqName(tag_name, "EventSource"))
		{
			is_set = true;
			text_element = element.firstChild().toText();
		}

		if (is_set)
		{
			unused_tag = false;
			setEventSource(text_element.data());
			--EventSource_count;
		}


		pElement = 0;
		is_set = false;
		attribute = "EventInitialRequestId";
		if (RootElement->hasAttribute(attribute))
		{
			pElement = RootElement;
		}
		else if (element.hasAttribute(attribute))
		{
			pElement = &element;
		}

		if (pElement && (EventInitialRequestId_count - 1) == 0)
		{
			text_element = temp_doc.createTextNode( pElement->attribute(attribute) );
			is_set = true;
		}
		else if (eqName(tag_name, "EventInitialRequestId"))
		{
			is_set = true;
			text_element = element.firstChild().toText();
		}

		if (is_set)
		{
			unused_tag = false;
			setInitRequestId(text_element.data());
			--EventInitialRequestId_count;
		}


		pElement = 0;
		is_set = false;
		attribute = "EventId";
		if (RootElement->hasAttribute(attribute))
		{
			pElement = RootElement;
		}
		else if (element.hasAttribute(attribute))
		{
			pElement = &element;
		}

		if (pElement && (EventId_count - 0) == 0)
		{
			text_element = temp_doc.createTextNode( pElement->attribute(attribute) );
			is_set = true;
		}
		else if (eqName(tag_name, "EventId"))
		{
			is_set = true;
			text_element = element.firstChild().toText();
		}

		if (is_set)
		{
			unused_tag = false;
			setEventId(text_element.data().toULongLong());
			--EventId_count;
		}


		if (eqName(tag_name, "EventParameters") && BaseEventParameters_count > 0)
		{
			unused_tag = false;
			CVmEventParameters* object = m_lstBaseEventParameters[1 - BaseEventParameters_count];
			object->makeFullItemId(getFullItemId(), "EventParameters");
			if (object->readXml(&element, tag_name, unite_with_loaded))
			{
				m_qsErrorMessage = object->GetErrorMessage();
				return PRL_ERR_PARSE_VM_CONFIG;
			}
			--BaseEventParameters_count;
			object->setSectionFakeFlag(false);
			m_lstWarningList += object->GetWarningList();
		}

		if ( unused_tag )
			m_mapExtDoc.insert(nElemIdx, m_doc.importNode(element, true).toElement());
		nElemIdx++;
		element = element.nextSiblingElement();
	}

	if (BaseEventParameters_count >= 1)
	{
		m_lstWarningList += "Warning in class 'CVmEventBase': tag 'EventParameters' is absent";
	}

	if (EventType_count > 0)
	{
		m_qsErrorMessage = "Error in class 'CVmEventBase': tag 'EventType' does not satisfy 'minOccurs = 1' condition";
		return PRL_ERR_PARSE_VM_CONFIG;
	}

	if (EventLevel_count > 0)
	{
		m_qsErrorMessage = "Error in class 'CVmEventBase': tag 'EventLevel' does not satisfy 'minOccurs = 1' condition";
		return PRL_ERR_PARSE_VM_CONFIG;
	}

	if (EventCode_count > 0)
	{
		m_qsErrorMessage = "Error in class 'CVmEventBase': tag 'EventCode' does not satisfy 'minOccurs = 1' condition";
		return PRL_ERR_PARSE_VM_CONFIG;
	}

	if (EventNeedResponse_count > 0)
	{
		m_qsErrorMessage = "Error in class 'CVmEventBase': tag 'EventNeedResponse' does not satisfy 'minOccurs = 1' condition";
		return PRL_ERR_PARSE_VM_CONFIG;
	}

	if (EventIssuerType_count > 0)
	{
		m_qsErrorMessage = "Error in class 'CVmEventBase': tag 'EventIssuerType' does not satisfy 'minOccurs = 1' condition";
		return PRL_ERR_PARSE_VM_CONFIG;
	}

	if (EventIssuerId_count > 0)
	{
		m_qsErrorMessage = "Error in class 'CVmEventBase': tag 'EventIssuerId' does not satisfy 'minOccurs = 1' condition";
		return PRL_ERR_PARSE_VM_CONFIG;
	}

	if (EventSource_count > 0)
	{
		m_qsErrorMessage = "Error in class 'CVmEventBase': tag 'EventSource' does not satisfy 'minOccurs = 1' condition";
		return PRL_ERR_PARSE_VM_CONFIG;
	}

	if (EventInitialRequestId_count > 0)
	{
		m_qsErrorMessage = "Error in class 'CVmEventBase': tag 'EventInitialRequestId' does not satisfy 'minOccurs = 1' condition";
		return PRL_ERR_PARSE_VM_CONFIG;
	}

	if (EventId_count >= 0)
	{
		m_lstWarningList += "Warning in class 'CVmEventBase': tag 'EventId' is absent";
	}



	return 0;
}

void CVmEventBase::syncItemIds()
{
	int i = -1;
	Q_UNUSED(i);
	int nMaxItemId = -1;
	Q_UNUSED(nMaxItemId);
	QSet<int > setItemIds;

	for(i = 0; i < m_lstBaseEventParameters.size(); ++i)
	{
		CVmEventParameters* object = m_lstBaseEventParameters[i];
		if ( ! object ) continue;
		object->makeFullItemId(getFullItemId(), "EventParameters");
		object->syncItemIds();
	}

}

bool CVmEventBase::merge(CVmEventBase* pCur, CVmEventBase* pPrev, MergeOptions nOptions)
{
	int i = -1;
	Q_UNUSED(i);
	Q_UNUSED(pCur);
	Q_UNUSED(pPrev);
	Q_UNUSED(nOptions);

	m_qsErrorMessage = "VirtuozzoEvent";

	if (getEventType() != pCur->getEventType() && getEventType() == pPrev->getEventType())
		setEventType(pCur->getEventType());
	else if (getEventType() != pCur->getEventType() && getEventType() != pPrev->getEventType() && pCur->getEventType() != pPrev->getEventType())
	{
		m_qsErrorMessage += ".EventType";
		return false;
	}

	if (getEventLevel() != pCur->getEventLevel() && getEventLevel() == pPrev->getEventLevel())
		setEventLevel(pCur->getEventLevel());
	else if (getEventLevel() != pCur->getEventLevel() && getEventLevel() != pPrev->getEventLevel() && pCur->getEventLevel() != pPrev->getEventLevel())
	{
		m_qsErrorMessage += ".EventLevel";
		return false;
	}

	if (getEventCode() != pCur->getEventCode() && getEventCode() == pPrev->getEventCode())
		setEventCode(pCur->getEventCode());
	else if (getEventCode() != pCur->getEventCode() && getEventCode() != pPrev->getEventCode() && pCur->getEventCode() != pPrev->getEventCode())
	{
		m_qsErrorMessage += ".EventCode";
		return false;
	}

	if (getRespRequired() != pCur->getRespRequired() && getRespRequired() == pPrev->getRespRequired())
		setRespRequired(pCur->getRespRequired());
	else if (getRespRequired() != pCur->getRespRequired() && getRespRequired() != pPrev->getRespRequired() && pCur->getRespRequired() != pPrev->getRespRequired())
	{
		m_qsErrorMessage += ".EventNeedResponse";
		return false;
	}

	if (getEventIssuerType() != pCur->getEventIssuerType() && getEventIssuerType() == pPrev->getEventIssuerType())
		setEventIssuerType(pCur->getEventIssuerType());
	else if (getEventIssuerType() != pCur->getEventIssuerType() && getEventIssuerType() != pPrev->getEventIssuerType() && pCur->getEventIssuerType() != pPrev->getEventIssuerType())
	{
		m_qsErrorMessage += ".EventIssuerType";
		return false;
	}

	if (getEventIssuerId() != pCur->getEventIssuerId() && getEventIssuerId() == pPrev->getEventIssuerId())
		setEventIssuerId(pCur->getEventIssuerId());
	else if (getEventIssuerId() != pCur->getEventIssuerId() && getEventIssuerId() != pPrev->getEventIssuerId() && pCur->getEventIssuerId() != pPrev->getEventIssuerId())
	{
		m_qsErrorMessage += ".EventIssuerId";
		return false;
	}

	if (getEventSource() != pCur->getEventSource() && getEventSource() == pPrev->getEventSource())
		setEventSource(pCur->getEventSource());
	else if (getEventSource() != pCur->getEventSource() && getEventSource() != pPrev->getEventSource() && pCur->getEventSource() != pPrev->getEventSource())
	{
		m_qsErrorMessage += ".EventSource";
		return false;
	}

	if (getInitRequestId() != pCur->getInitRequestId() && getInitRequestId() == pPrev->getInitRequestId())
		setInitRequestId(pCur->getInitRequestId());
	else if (getInitRequestId() != pCur->getInitRequestId() && getInitRequestId() != pPrev->getInitRequestId() && pCur->getInitRequestId() != pPrev->getInitRequestId())
	{
		m_qsErrorMessage += ".EventInitialRequestId";
		return false;
	}

	if (getEventId() != pCur->getEventId() && getEventId() == pPrev->getEventId())
		setEventId(pCur->getEventId());
	else if (getEventId() != pCur->getEventId() && getEventId() != pPrev->getEventId() && pCur->getEventId() != pPrev->getEventId())
	{
		m_qsErrorMessage += ".EventId";
		return false;
	}

	for(i = 0; i < m_lstBaseEventParameters.size(); i++)
	{
		if ( m_lstBaseEventParameters[i]->merge(pCur->m_lstBaseEventParameters[i], pPrev->m_lstBaseEventParameters[i], nOptions) ) continue;
		QString qsIdx = (m_lstBaseEventParameters.size() > 1 ? ("[" + QString::number(i) + "]") : QString());
		m_qsErrorMessage += "." + qsIdx + m_lstBaseEventParameters[i]->GetErrorMessage();
		return false;
	}

	return true;
}

void CVmEventBase::diff(const CVmEventBase* pOld, QStringList& lstDiffFullItemIds) const
{
	int i = -1;
	Q_UNUSED(i);
	Q_UNUSED(pOld);
	Q_UNUSED(lstDiffFullItemIds);

	if (getEventType() != pOld->getEventType())
		lstDiffFullItemIds += getEventType_id();

	if (getEventLevel() != pOld->getEventLevel())
		lstDiffFullItemIds += getEventLevel_id();

	if (getEventCode() != pOld->getEventCode())
		lstDiffFullItemIds += getEventCode_id();

	if (getRespRequired() != pOld->getRespRequired())
		lstDiffFullItemIds += getRespRequired_id();

	if (getEventIssuerType() != pOld->getEventIssuerType())
		lstDiffFullItemIds += getEventIssuerType_id();

	if (getEventIssuerId() != pOld->getEventIssuerId())
		lstDiffFullItemIds += getEventIssuerId_id();

	if (getEventSource() != pOld->getEventSource())
		lstDiffFullItemIds += getEventSource_id();

	if (getInitRequestId() != pOld->getInitRequestId())
		lstDiffFullItemIds += getInitRequestId_id();

	if (getEventId() != pOld->getEventId())
		lstDiffFullItemIds += getEventId_id();

	for(i = 0; i < m_lstBaseEventParameters.size(); ++i)
		m_lstBaseEventParameters[i]->diff(pOld->m_lstBaseEventParameters[i], lstDiffFullItemIds);

}

void CVmEventBase::Copy(const CVmEventBase& rObject)
{
	if (this == &rObject)
		return;

	int i = -1;
	Q_UNUSED(i);

	CopyBase(&rObject);

	setEventType(rObject.getEventType());
	setEventLevel(rObject.getEventLevel());
	setEventCode(rObject.getEventCode());
	setRespRequired(rObject.getRespRequired());
	setEventIssuerType(rObject.getEventIssuerType());
	setEventIssuerId(rObject.getEventIssuerId());
	setEventSource(rObject.getEventSource());
	setInitRequestId(rObject.getInitRequestId());
	setEventId(rObject.getEventId());

	ClearList<CVmEventParameters >(m_lstBaseEventParameters);
	for(i = 0; i < rObject.m_lstBaseEventParameters.size(); i++)
		m_lstBaseEventParameters += new CVmEventParameters(rObject.m_lstBaseEventParameters[i]);

	CustomCopy(&rObject);

}

void CVmEventBase::InitLists()
{
	InitList<CVmEventParameters>(1, m_lstBaseEventParameters);
	CustomInit();
}

void CVmEventBase::ClearLists()
{
	ClearList<CVmEventParameters>(m_lstBaseEventParameters);
}

void CVmEventBase::ClearListsInReadXml(bool unite_with_loaded, const QStringList& dyn_lists, bool bSupportDynList)
{
	Q_UNUSED(unite_with_loaded);
	Q_UNUSED(dyn_lists);
	Q_UNUSED(bSupportDynList);
	QDomElement element;
	int i = -1;
	Q_UNUSED(i);
	for(i = 0; i < m_lstBaseEventParameters.size() && ! unite_with_loaded; i++)
	{
		m_lstBaseEventParameters[i]->ClearListsInReadXml();
		m_lstBaseEventParameters[i]->setDefaults();
		m_lstBaseEventParameters[i]->setSectionFakeFlag(true);
	}
}

