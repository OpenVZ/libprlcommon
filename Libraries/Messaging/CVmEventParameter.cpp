/*
 * CVmEventParameter.cpp
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

#include "CVmEventParameter.h"


CVmEventParameter::CVmEventParameter()
{
	InitLists();
	setDefaults();
}

CVmEventParameter::CVmEventParameter(const CVmEventParameter& rObject)
: CPrlDataSerializer(), CBaseNode()
{
	Copy(rObject);
}

CVmEventParameter::CVmEventParameter(const CVmEventParameter* pObject)
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

CVmEventParameter& CVmEventParameter::operator=(const CVmEventParameter& rObject)
{
	Copy(rObject);
	return *this;
}

CVmEventParameter::CVmEventParameter(QFile* pFile)
{
	InitLists();
	loadFromFile(pFile);
}

CVmEventParameter::~CVmEventParameter()
{
	ClearLists();
}


void CVmEventParameter::Serialize(QDataStream &_stream)
{
    PrlOpaqueSerializer((quint32 &)PrlOpaqueTypeConverter(m_uiEventParameterClassType)).Serialize(_stream);
    PrlOpaqueSerializer((quint32 &)PrlOpaqueTypeConverter(m_ctType)).Serialize(_stream);

    QString strParamValue = getParamValue();
    CPrlStringDataSerializer(strParamValue).Serialize(_stream);
    CPrlStringDataSerializer(m_qsName).Serialize(_stream);

    QString strCData = getCdata();
    CPrlStringDataSerializer(strCData).Serialize(_stream);
}

void CVmEventParameter::Deserialize(QDataStream &_stream)
{
    cleanupClassProperties();
    PrlOpaqueSerializer((quint32 &)PrlOpaqueTypeConverter(m_ctType)).Deserialize(_stream);
    QString strParamValue;
    CPrlStringDataSerializer(strParamValue).Deserialize(_stream);
    setParamValue(strParamValue);
    CPrlStringDataSerializer(m_qsName).Deserialize(_stream);
    QString strCData;
    CPrlStringDataSerializer(strCData).Deserialize(_stream);
    setCdata(strCData);
}

void CVmEventParameter::CustomCopy(const CBaseNode* pBN)
{
    if ( ! pBN ) return;

    setEventParameterClassType( ((CVmEventParameter* )pBN)->getEventParameterClassType() );
}
void CVmEventParameter::CustomInit()
{
    m_uiEventParameterClassType = BaseType;
}

CVmEventParameter::EventParameterClassType CVmEventParameter::getEventParameterClassType() const
{
	return (m_uiEventParameterClassType);
}

void CVmEventParameter::setEventParameterClassType(EventParameterClassType type)
{
    m_uiEventParameterClassType = type;
}

QStringList CVmEventParameter::getValuesList()
{
    if ( isList() && !m_lstValues.isEmpty() )
    {
        return m_lstValues[0]->getListItem();
    }
    return QStringList();
}

bool CVmEventParameter::isListParameter(QDomElement *xmlItemElement)
{
	QDomElement xmlParamListSignElem = xmlItemElement->firstChildElement(XML_VM_ND_IS_LIST);
	return (!xmlParamListSignElem.isNull());
}

CVmEventParameter::CVmEventParameter(PVE::ParamFieldDataType param_type,
	QString param_value, QString param_name)
{
	cleanupClassProperties();

	m_bIsList = false;
	m_uiEventParameterClassType = BaseType;

//	if( isCorrectParamType( param_type ) )
		setParamType( param_type );
//	else
//		m_uiType = PVE::String;

	m_qsName = param_name;

	if(getParamType() == PVE::CData)
		m_baData = param_value.toUtf8();
	else if (!m_lstValues.isEmpty())
		setParamValue(param_value);
}

CVmEventValue* CVmEventParameter::getValue() const
{
	if (!m_lstValues.isEmpty())
	{
		return m_lstValues[0];
	}
	return 0;
}

void CVmEventParameter::setValue(CVmEventValue* pValues)
{
	ClearList<CVmEventValue>(m_lstValues);
	if (pValues)
	{
		m_lstValues += pValues;
	}
}

bool CVmEventParameter::isIsList() const
{
	return m_bIsList;
}

void CVmEventParameter::setIsList(bool value)
{
	m_bIsList = value;
}

QString CVmEventParameter::getParamName() const
{
	return m_qsName;
}

void CVmEventParameter::setParamName(QString value)
{
	m_qsName = value;
}

PVE::ParamFieldDataType CVmEventParameter::getParamType() const
{
	return m_ctType;
}

void CVmEventParameter::setParamType(PVE::ParamFieldDataType value)
{
	m_ctType = value;
}

QString CVmEventParameter::getParamValue() const
{
	return m_qsValue;
}

void CVmEventParameter::setParamValue(QString value)
{
	m_qsValue = value;
}

QByteArray CVmEventParameter::getData() const
{
	return m_baData;
}

void CVmEventParameter::setData(QByteArray value)
{
	m_baData = value;
}

void CVmEventParameter::setDefaults(QDomElement* RootElement)
{
	Q_UNUSED(RootElement);

	QStringList dyn_lists;
	if (RootElement)
		dyn_lists = RootElement->attribute("dyn_lists").split(" ");

	if ( ! RootElement || ! RootElement->firstChildElement("IsList").isNull() )
		setIsList();
	if ( ! RootElement || ! RootElement->firstChildElement("Name").isNull() )
		setParamName();
	if ( ! RootElement || ! RootElement->firstChildElement("Type").isNull() )
		setParamType();
	if ( ! RootElement || ! RootElement->firstChildElement("Value").isNull() )
		setParamValue();
	if ( ! RootElement || ! RootElement->firstChildElement("Data").isNull() )
		setData();

	static bool first_time = true;
	if ( first_time )
	{
		qRegisterMetaType< PVE::ParamFieldDataType >("PVE::ParamFieldDataType");
		first_time = false;
	}

}

QVariant CVmEventParameter::getPropertyValue(QString path) const
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

	if (path == "IsList.patch_stamp")
		value.setValue(getFieldPatchedValue("IsList"));

	if (path == "IsList")
		value.setValue(isIsList());

	if (path == "Name.patch_stamp")
		value.setValue(getFieldPatchedValue("Name"));

	if (path == "Name")
		value.setValue(getParamName());

	if (path == "Type.patch_stamp")
		value.setValue(getFieldPatchedValue("Type"));

	if (path == "Type")
		value.setValue((qlonglong )getParamType());

	if (path == "Value.patch_stamp")
		value.setValue(getFieldPatchedValue("Value"));

	if (path == "Value")
		value.setValue(getParamValue());

	if (path == "Data.patch_stamp")
		value.setValue(getFieldPatchedValue("Data"));

	if (path == "Data")
		value.setValue(getData());

	list_tag = "Value.";
	if (path.startsWith(list_tag) && ! m_lstValues.isEmpty() && m_lstValues.first())
		return m_lstValues.first()->getPropertyValue(path.mid(list_tag.size()));

	return value;
}

bool CVmEventParameter::setPropertyValue(QString path, QVariant value, bool* pbValueChanged)
{
	QString list_tag;
	bool bOk = false;
	Q_UNUSED(path);
	Q_UNUSED(value);
	Q_UNUSED(pbValueChanged);
	Q_UNUSED(list_tag);
	Q_UNUSED(bOk);

	if (path == "IsList.patch_stamp")
	{
		markPatchedField("IsList", value.toString());
		return true;
	}

	if (path == "IsList")
	{
		if (pbValueChanged)
			*pbValueChanged = (isIsList() != value.value< bool >());
		setIsList(value.value< bool >());
		return true;
	}

	if (path == "Name.patch_stamp")
	{
		markPatchedField("Name", value.toString());
		return true;
	}

	if (path == "Name")
	{
		if (pbValueChanged)
			*pbValueChanged = (getParamName() != value.value< QString >());
		setParamName(value.value< QString >());
		return true;
	}

	if (path == "Type.patch_stamp")
	{
		markPatchedField("Type", value.toString());
		return true;
	}

	if (path == "Type")
	{
		if (pbValueChanged)
			*pbValueChanged = (getParamType() != (PVE::ParamFieldDataType )value.value< qlonglong >());
		setParamType((PVE::ParamFieldDataType )value.value< qlonglong >());
		return true;
	}

	if (path == "Value.patch_stamp")
	{
		markPatchedField("Value", value.toString());
		return true;
	}

	if (path == "Value")
	{
		if (pbValueChanged)
			*pbValueChanged = (getParamValue() != value.value< QString >());
		setParamValue(value.value< QString >());
		return true;
	}

	if (path == "Data.patch_stamp")
	{
		markPatchedField("Data", value.toString());
		return true;
	}

	if (path == "Data")
	{
		if (pbValueChanged)
			*pbValueChanged = (getData() != value.value< QByteArray >());
		setData(value.value< QByteArray >());
		return true;
	}

	list_tag = "Value.";
	if (path.startsWith(list_tag) && ! m_lstValues.isEmpty() && m_lstValues.first())
		return m_lstValues.first()->setPropertyValue(path.mid(list_tag.size()), value, pbValueChanged);

	return false;
}

QString CVmEventParameter::getIsList_id() const
{
	return getFullItemId().isEmpty() ? "IsList" : getFullItemId() + ".IsList";
}
QString CVmEventParameter::getParamName_id() const
{
	return getFullItemId().isEmpty() ? "Name" : getFullItemId() + ".Name";
}
QString CVmEventParameter::getParamType_id() const
{
	return getFullItemId().isEmpty() ? "Type" : getFullItemId() + ".Type";
}
QString CVmEventParameter::getParamValue_id() const
{
	return getFullItemId().isEmpty() ? "Value" : getFullItemId() + ".Value";
}
QString CVmEventParameter::getData_id() const
{
	return getFullItemId().isEmpty() ? "Data" : getFullItemId() + ".Data";
}

int CVmEventParameter::addListItem(QString path)
{
	int nNewItemId = -1;
	QString list_tag;
	bool bOk = false;
	Q_UNUSED(path);
	Q_UNUSED(nNewItemId);
	Q_UNUSED(list_tag);
	Q_UNUSED(bOk);

	list_tag = "Value.";
	if (path.startsWith(list_tag) && ! m_lstValues.isEmpty() && m_lstValues.first())
		return m_lstValues.first()->addListItem(path.mid(list_tag.size()));

	return nNewItemId;
}

bool CVmEventParameter::deleteListItem(QString path)
{
	QString list_tag;
	bool bOk = false;
	Q_UNUSED(path);
	Q_UNUSED(list_tag);
	Q_UNUSED(bOk);

	list_tag = "Value.";
	if (path.startsWith(list_tag) && ! m_lstValues.isEmpty() && m_lstValues.first())
		return m_lstValues.first()->deleteListItem(path.mid(list_tag.size()));

	return false;
}

QDomElement CVmEventParameter::getXml(QDomDocument* Document, bool no_save_option) const
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

	QDomElement root_element = Document->createElement(m_qsExtRootTagName.isEmpty() ? QString("EventParameter") : m_qsExtRootTagName);

	if (isList())
	{
		checkAndInsertExtDocElement(root_element, nElemIdx);
		element = Document->createElement("IsList");
		text_element = Document->createTextNode( QString("%1").arg(isIsList()) );
		root_element.appendChild(element);
		element.appendChild(text_element);

	}

	checkAndInsertExtDocElement(root_element, nElemIdx);
	element = Document->createElement("Name");
	text_element = Document->createTextNode( QString("%1").arg(getParamName()) );
	root_element.appendChild(element);
	element.appendChild(text_element);

	checkAndInsertExtDocElement(root_element, nElemIdx);
	element = Document->createElement("Type");
	text_element = Document->createTextNode( QString("%1").arg(getParamType()) );
	root_element.appendChild(element);
	element.appendChild(text_element);

	if (!isList())
	{
		checkAndInsertExtDocElement(root_element, nElemIdx);
		element = Document->createElement("Value");
		text_element = Document->createTextNode( QString("%1").arg(getParamValue()) );
		root_element.appendChild(element);
		element.appendChild(text_element);

	}

	for(i = 0; i < m_lstValues.size() && (isList()); ++i)
	{
		checkAndInsertExtDocElement(root_element, nElemIdx);
		const CVmEventValue* object = m_lstValues[i];
		if ( ! object ) continue;
		element = object->getXml(Document, no_save_option);
		root_element.appendChild(element);
	}

	if (getParamType() == PVE::CData || getParamType() == PVE::VmConfiguration)
	{
		checkAndInsertExtDocElement(root_element, nElemIdx);
		element = Document->createElement("Data");
		text_element = Document->createTextNode( QString("%1").arg(getData().toBase64().data()) );
		root_element.appendChild(element);
		element.appendChild(text_element);

	}

	checkAndInsertExtDocElement(root_element, nElemIdx);

	root_element.setAttribute("dyn_lists", dyn_lists.join(" "));

	return root_element;
}

int CVmEventParameter::readXml(QDomElement* RootElement, QString ext_tag_name, bool unite_with_loaded)
{
	QDomDocument temp_doc;
	QString tag_name;
	QString attribute;
	QDomText text_element;
	QDomCDATASection cdata_element;

	tag_name = RootElement->tagName();
	m_qsTagName = tag_name;
	m_qsExtRootTagName = ext_tag_name;
	if (tag_name != (ext_tag_name.isEmpty() ? QString("EventParameter") : ext_tag_name))
	{
		m_qsErrorMessage = "Error in class 'CVmEventParameter': wrong root element with tag name '" + tag_name + "'";
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

	int Values_count = 1;
	int IsList_count = 0;
	int Name_count = 1;
	int Type_count = 1;
	int Value_count = 0;
	int Data_count = 0;

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
		attribute = "IsList";
		if (RootElement->hasAttribute(attribute))
		{
			pElement = RootElement;
		}
		else if (element.hasAttribute(attribute))
		{
			pElement = &element;
		}

		if (pElement && (IsList_count - 0) == 0)
		{
			text_element = temp_doc.createTextNode( pElement->attribute(attribute) );
			is_set = true;
		}
		else if (tag_name == "IsList")
		{
			is_set = true;
			text_element = element.firstChild().toText();
		}

		if (is_set)
		{
			unused_tag = false;
			setIsList(text_element.data().toInt() != 0);
			--IsList_count;
		}


		pElement = 0;
		is_set = false;
		attribute = "Name";
		if (RootElement->hasAttribute(attribute))
		{
			pElement = RootElement;
		}
		else if (element.hasAttribute(attribute))
		{
			pElement = &element;
		}

		if (pElement && (Name_count - 1) == 0)
		{
			text_element = temp_doc.createTextNode( pElement->attribute(attribute) );
			is_set = true;
		}
		else if (tag_name == "Name")
		{
			is_set = true;
			text_element = element.firstChild().toText();
		}

		if (is_set)
		{
			unused_tag = false;
			setParamName(text_element.data());
			--Name_count;
		}


		pElement = 0;
		is_set = false;
		attribute = "Type";
		if (RootElement->hasAttribute(attribute))
		{
			pElement = RootElement;
		}
		else if (element.hasAttribute(attribute))
		{
			pElement = &element;
		}

		if (pElement && (Type_count - 1) == 0)
		{
			text_element = temp_doc.createTextNode( pElement->attribute(attribute) );
			is_set = true;
		}
		else if (tag_name == "Type")
		{
			is_set = true;
			text_element = element.firstChild().toText();
		}

		if (is_set)
		{
			unused_tag = false;
			setParamType((PVE::ParamFieldDataType )text_element.data().toLongLong());
			--Type_count;
		}


		pElement = 0;
		is_set = false;
		attribute = "Value";
		if (RootElement->hasAttribute(attribute))
		{
			pElement = RootElement;
		}
		else if (element.hasAttribute(attribute))
		{
			pElement = &element;
		}

		if (pElement && (Value_count - 0) == 0)
		{
			text_element = temp_doc.createTextNode( pElement->attribute(attribute) );
			is_set = true;
		}
		else if (tag_name == "Value")
		{
			is_set = true;
			text_element = element.firstChild().toText();
		}

		if (is_set)
		{
			unused_tag = false;
			setParamValue(text_element.data());
			--Value_count;
		}


		pElement = 0;
		is_set = false;
		attribute = "Data";
		if (RootElement->hasAttribute(attribute))
		{
			pElement = RootElement;
		}
		else if (element.hasAttribute(attribute))
		{
			pElement = &element;
		}

		if (pElement && (Data_count - 0) == 0)
		{
			text_element = temp_doc.createTextNode( pElement->attribute(attribute) );
			is_set = true;
		}
		else if (tag_name == "Data")
		{
			is_set = true;
			text_element = element.firstChild().toText();
		}

		if (is_set)
		{
			unused_tag = false;
			setData(QByteArray::fromBase64( text_element.data().toLatin1() ));
			--Data_count;
		}


		if (tag_name == "Value" && Values_count > 0)
		{
			unused_tag = false;
			CVmEventValue* object = m_lstValues[1 - Values_count];
			object->makeFullItemId(getFullItemId(), "Value");
			if (object->readXml(&element, tag_name, unite_with_loaded))
			{
				m_qsErrorMessage = object->GetErrorMessage();
				return PRL_ERR_PARSE_VM_CONFIG;
			}
			--Values_count;
			object->setSectionFakeFlag(false);
			m_lstWarningList += object->GetWarningList();
		}

		if ( unused_tag )
			m_mapExtDoc.insert(nElemIdx, m_doc.importNode(element, true).toElement());
		nElemIdx++;
		element = element.nextSiblingElement();
	}

	if (Values_count >= 1)
	{
		m_lstWarningList += "Warning in class 'CVmEventParameter': tag 'Value' is absent";
	}

	if (IsList_count >= 0)
	{
		m_lstWarningList += "Warning in class 'CVmEventParameter': tag 'IsList' is absent";
	}

	if (Name_count > 0)
	{
		m_qsErrorMessage = "Error in class 'CVmEventParameter': tag 'Name' does not satisfy 'minOccurs = 1' condition";
		return PRL_ERR_PARSE_VM_CONFIG;
	}

	if (Type_count > 0)
	{
		m_qsErrorMessage = "Error in class 'CVmEventParameter': tag 'Type' does not satisfy 'minOccurs = 1' condition";
		return PRL_ERR_PARSE_VM_CONFIG;
	}

	if (Value_count >= 0)
	{
		m_lstWarningList += "Warning in class 'CVmEventParameter': tag 'Value' is absent";
	}

	if (Data_count >= 0)
	{
		m_lstWarningList += "Warning in class 'CVmEventParameter': tag 'Data' is absent";
	}



	return 0;
}

void CVmEventParameter::syncItemIds()
{
	int i = -1;
	Q_UNUSED(i);
	int nMaxItemId = -1;
	Q_UNUSED(nMaxItemId);
	QSet<int > setItemIds;

	for(i = 0; i < m_lstValues.size(); ++i)
	{
		CVmEventValue* object = m_lstValues[i];
		if ( ! object ) continue;
		object->makeFullItemId(getFullItemId(), "Value");
		object->syncItemIds();
	}

}

bool CVmEventParameter::merge(CVmEventParameter* pCur, CVmEventParameter* pPrev, MergeOptions nOptions)
{
	int i = -1;
	Q_UNUSED(i);
	Q_UNUSED(pCur);
	Q_UNUSED(pPrev);
	Q_UNUSED(nOptions);

	m_qsErrorMessage = "EventParameter";

	if (isIsList() != pCur->isIsList() && isIsList() == pPrev->isIsList())
		setIsList(pCur->isIsList());
	else if (isIsList() != pCur->isIsList() && isIsList() != pPrev->isIsList() && pCur->isIsList() != pPrev->isIsList())
	{
		m_qsErrorMessage += ".IsList";
		return false;
	}

	if (getParamName() != pCur->getParamName() && getParamName() == pPrev->getParamName())
		setParamName(pCur->getParamName());
	else if (getParamName() != pCur->getParamName() && getParamName() != pPrev->getParamName() && pCur->getParamName() != pPrev->getParamName())
	{
		m_qsErrorMessage += ".Name";
		return false;
	}

	if (getParamType() != pCur->getParamType() && getParamType() == pPrev->getParamType())
		setParamType(pCur->getParamType());
	else if (getParamType() != pCur->getParamType() && getParamType() != pPrev->getParamType() && pCur->getParamType() != pPrev->getParamType())
	{
		m_qsErrorMessage += ".Type";
		return false;
	}

	if (getParamValue() != pCur->getParamValue() && getParamValue() == pPrev->getParamValue())
		setParamValue(pCur->getParamValue());
	else if (getParamValue() != pCur->getParamValue() && getParamValue() != pPrev->getParamValue() && pCur->getParamValue() != pPrev->getParamValue())
	{
		m_qsErrorMessage += ".Value";
		return false;
	}

	if (getData() != pCur->getData() && getData() == pPrev->getData())
		setData(pCur->getData());
	else if (getData() != pCur->getData() && getData() != pPrev->getData() && pCur->getData() != pPrev->getData())
	{
		m_qsErrorMessage += ".Data";
		return false;
	}

	for(i = 0; i < m_lstValues.size(); i++)
	{
		if ( m_lstValues[i]->merge(pCur->m_lstValues[i], pPrev->m_lstValues[i], nOptions) ) continue;
		QString qsIdx = (m_lstValues.size() > 1 ? ("[" + QString::number(i) + "]") : QString());
		m_qsErrorMessage += "." + qsIdx + m_lstValues[i]->GetErrorMessage();
		return false;
	}

	return true;
}

void CVmEventParameter::diff(const CVmEventParameter* pOld, QStringList& lstDiffFullItemIds) const
{
	int i = -1;
	Q_UNUSED(i);
	Q_UNUSED(pOld);
	Q_UNUSED(lstDiffFullItemIds);

	if (isIsList() != pOld->isIsList())
		lstDiffFullItemIds += getIsList_id();

	if (getParamName() != pOld->getParamName())
		lstDiffFullItemIds += getParamName_id();

	if (getParamType() != pOld->getParamType())
		lstDiffFullItemIds += getParamType_id();

	if (getParamValue() != pOld->getParamValue())
		lstDiffFullItemIds += getParamValue_id();

	if (getData() != pOld->getData())
		lstDiffFullItemIds += getData_id();

	for(i = 0; i < m_lstValues.size(); ++i)
		m_lstValues[i]->diff(pOld->m_lstValues[i], lstDiffFullItemIds);

}

void CVmEventParameter::Copy(const CVmEventParameter& rObject)
{
	if (this == &rObject)
		return;

	int i = -1;
	Q_UNUSED(i);

	CopyBase(&rObject);

	setIsList(rObject.isIsList());
	setParamName(rObject.getParamName());
	setParamType(rObject.getParamType());
	setParamValue(rObject.getParamValue());
	setData(rObject.getData());

	ClearList<CVmEventValue >(m_lstValues);
	for(i = 0; i < rObject.m_lstValues.size(); i++)
		m_lstValues += new CVmEventValue(rObject.m_lstValues[i]);

	CustomCopy(&rObject);

}

void CVmEventParameter::InitLists()
{
	InitList<CVmEventValue>(1, m_lstValues);
	CustomInit();
}

void CVmEventParameter::ClearLists()
{
	ClearList<CVmEventValue>(m_lstValues);
}

void CVmEventParameter::ClearListsInReadXml(bool unite_with_loaded, const QStringList& dyn_lists, bool bSupportDynList)
{
	Q_UNUSED(unite_with_loaded);
	Q_UNUSED(dyn_lists);
	Q_UNUSED(bSupportDynList);
	QDomElement element;
	int i = -1;
	Q_UNUSED(i);
	for(i = 0; i < m_lstValues.size() && ! unite_with_loaded; i++)
	{
		m_lstValues[i]->ClearListsInReadXml();
		m_lstValues[i]->setDefaults();
		m_lstValues[i]->setSectionFakeFlag(true);
	}
}

