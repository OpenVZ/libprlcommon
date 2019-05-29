/*
 * CVmEventBase.h: Definition of the class CVmEventBase. This class
 * implements base of VM Event.
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

#ifndef CVMEVENTBASE_H
#define CVMEVENTBASE_H


#include "../PrlObjects/CBaseNode.h"
#include "CVmEventParameters.h"


class CVmEventBase : public CPrlDataSerializer
				, public CBaseNode
{

public:

	CVmEventBase();
	CVmEventBase(const CVmEventBase& rObject);
	CVmEventBase(const CVmEventBase* pObject);
	CVmEventBase(QFile* pFile);
	CVmEventBase& operator=(const CVmEventBase& rObject);
	virtual ~CVmEventBase();


	QList<CVmEventParameters* >		m_lstBaseEventParameters;

	CVmEventParameters* getEventParameters() const;
	void setEventParameters(CVmEventParameters* pBaseEventParameters);


	PRL_EVENT_TYPE getEventType() const;
	void setEventType(PRL_EVENT_TYPE value = PET_VM_INF_UNINITIALIZED_EVENT_CODE);

	PVE::VmEventLevel getEventLevel() const;
	void setEventLevel(PVE::VmEventLevel value = PVE::EventLevel0);

	PRL_RESULT getEventCode() const;
	void setEventCode(PRL_RESULT value = PRL_ERR_SUCCESS);

	PVE::VmEventRespOption getRespRequired() const;
	void setRespRequired(PVE::VmEventRespOption value = PVE::EventRespNotRequired);

	PRL_EVENT_ISSUER_TYPE getEventIssuerType() const;
	void setEventIssuerType(PRL_EVENT_ISSUER_TYPE value = PIE_VIRTUAL_MACHINE);

	QString getEventIssuerId() const;
	void setEventIssuerId(QString value = QString());

	QString getEventSource() const;
	void setEventSource(QString value = QString());

	QString getInitRequestId() const;
	void setInitRequestId(QString value = QString());

	qulonglong getEventId() const;
	void setEventId(qulonglong value = 0);

	virtual void setDefaults(QDomElement* RootElement = 0);

	virtual QVariant getPropertyValue(QString path) const;
	virtual bool setPropertyValue(QString path, QVariant value, bool* pbValueChanged = 0);

	QString getEventType_id() const;
	QString getEventLevel_id() const;
	QString getEventCode_id() const;
	QString getRespRequired_id() const;
	QString getEventIssuerType_id() const;
	QString getEventIssuerId_id() const;
	QString getEventSource_id() const;
	QString getInitRequestId_id() const;
	QString getEventId_id() const;

	int addListItem(QString path);
	bool deleteListItem(QString path);

	virtual QDomElement getXml(QDomDocument* Document, bool no_save_option = false) const;
	virtual int readXml(QDomElement* RootElement, QString ext_tag_name = QString(), bool unite_with_loaded = false);
	virtual void syncItemIds();

	bool merge(CVmEventBase* pCur, CVmEventBase* pPrev, MergeOptions nOptions);

	void diff(const CVmEventBase* pOld, QStringList& lstDiffFullItemIds) const;

	virtual void InitLists();
	virtual void ClearLists();
	virtual void ClearListsInReadXml(bool unite_with_loaded = false, const QStringList& dyn_lists = QStringList(), bool bSupportDynList = true);

protected:

	PRL_EVENT_TYPE		m_ctEventType;
	PVE::VmEventLevel		m_ctEventLevel;
	PRL_RESULT		m_ctEventCode;
	PVE::VmEventRespOption		m_ctEventNeedResponse;
	PRL_EVENT_ISSUER_TYPE		m_ctEventIssuerType;
	QString		m_qsEventIssuerId;
	QString		m_qsEventSource;
	QString		m_qsEventInitialRequestId;
	qulonglong		m_ullEventId;

	void Copy(const CVmEventBase& rObject);

};

#ifndef DM_233037509
#define DM_233037509
Q_DECLARE_METATYPE(PRL_EVENT_TYPE)
#endif
#ifndef DM_191362780
#define DM_191362780
Q_DECLARE_METATYPE(PVE::VmEventLevel)
#endif
#ifndef DM_73410644
#define DM_73410644
Q_DECLARE_METATYPE(PRL_RESULT)
#endif
#ifndef DM_34214494
#define DM_34214494
Q_DECLARE_METATYPE(PVE::VmEventRespOption)
#endif
#ifndef DM_118594565
#define DM_118594565
Q_DECLARE_METATYPE(PRL_EVENT_ISSUER_TYPE)
#endif


#endif
