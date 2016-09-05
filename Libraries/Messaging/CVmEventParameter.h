/*
 * CVmEventParameter.h: Definition of the class CVmEventParameter. This class
 * implements generic VM Event Parameter.
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
#ifndef CVMEVENTPARAMETER_H
#define CVMEVENTPARAMETER_H


#include "../PrlObjects/CBaseNode.h"
#include "CVmEventValue.h"


class CVmEventParameter : public CPrlDataSerializer
				, public CBaseNode
{

public:

	CVmEventParameter();
	CVmEventParameter(const CVmEventParameter& rObject);
	CVmEventParameter(const CVmEventParameter* pObject);
	CVmEventParameter(QFile* pFile);
	CVmEventParameter& operator=(const CVmEventParameter& rObject);
	virtual ~CVmEventParameter();


public:
	virtual void Serialize(QDataStream &_stream);
	virtual void Deserialize(QDataStream &_stream);

    virtual void CustomCopy(const CBaseNode* pBN);
    virtual void CustomInit();

    enum EventParameterClassType
    {
        BaseType = 0,
        List,
        Binary
    };

    EventParameterClassType getEventParameterClassType() const;
    void setEventParameterClassType(EventParameterClassType type);
protected:
    EventParameterClassType m_uiEventParameterClassType;
public:

    QStringList getValuesList();

    void setCdata(QString value) { setData(value.toUtf8()); }
    QString getCdata() { return UTF8_2QSTR(getData().data()); }

	static bool isListParameter(QDomElement *xmlItemElement);

	virtual bool isList() const
	{
		return isIsList();
	}

	CVmEventParameter(PVE::ParamFieldDataType param_type, QString param_value, QString param_name = "");

	QList<CVmEventValue* >		m_lstValues;

	CVmEventValue* getValue() const;
	void setValue(CVmEventValue* pValues);


	bool isIsList() const;
	void setIsList(bool value = false);

	QString getParamName() const;
	void setParamName(QString value = QString());

	PVE::ParamFieldDataType getParamType() const;
	void setParamType(PVE::ParamFieldDataType value = PVE::String);

	QString getParamValue() const;
	void setParamValue(QString value = QString());

	QByteArray getData() const;
	void setData(QByteArray value = QByteArray());

	virtual void setDefaults(QDomElement* RootElement = 0);

	virtual QVariant getPropertyValue(QString path) const;
	virtual bool setPropertyValue(QString path, QVariant value, bool* pbValueChanged = 0);

	QString getIsList_id() const;
	QString getParamName_id() const;
	QString getParamType_id() const;
	QString getParamValue_id() const;
	QString getData_id() const;

	int addListItem(QString path);
	bool deleteListItem(QString path);

	virtual QDomElement getXml(QDomDocument* Document, bool no_save_option = false) const;
	virtual int readXml(QDomElement* RootElement, QString ext_tag_name = QString(), bool unite_with_loaded = false);
	virtual void syncItemIds();

	bool merge(CVmEventParameter* pCur, CVmEventParameter* pPrev, MergeOptions nOptions);

	void diff(const CVmEventParameter* pOld, QStringList& lstDiffFullItemIds) const;

	virtual void InitLists();
	virtual void ClearLists();
	virtual void ClearListsInReadXml(bool unite_with_loaded = false, const QStringList& dyn_lists = QStringList(), bool bSupportDynList = true);

protected:

	bool		m_bIsList;
	QString		m_qsName;
	PVE::ParamFieldDataType		m_ctType;
	QString		m_qsValue;
	QByteArray		m_baData;

	void Copy(const CVmEventParameter& rObject);

};

#ifndef DM_240760741
#define DM_240760741
Q_DECLARE_METATYPE(PVE::ParamFieldDataType)
#endif

typedef std::vector< CVmEventParameter* > VmEventParamVector;

#endif
