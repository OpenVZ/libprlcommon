/*
 * Copyright (c) 2015 Parallels IP Holdings GmbH
 *
 * This file is part of Virtuozzo Core. Virtuozzo Core is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation;
 * either version 2 of the License, or (at your option) any later
 * version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 * Our contact details: Parallels IP Holdings GmbH, Vordergasse 59, 8200
 * Schaffhausen, Switzerland.
 */


#include <QApplication>

#include <QBuffer>
#include <QByteArray>
#include <QDataStream>
#include "Libraries/Logging/Logging.h"

#include "Server.h"

#ifdef _WIN_
 #include <windows.h>
 #define sleepMsecs Sleep
#else
 #include <unistd.h>
 #define sleepMsecs(usecs) usleep(usecs * 1000)
#endif

using namespace IOService;

/*****************************************************************************/

Server::Server () :
    m_server( 0 ),
    m_secLevel( PSL_LOW_SECURITY ),
    m_host( IOService::LoopbackAddr )
{
    setupUi(this);

    // Default init
    statusBar()->showMessage(tr("Disconnected"));
    sendButton->setDisabled( true );
    hostEdit->setDisabled( true );
    connUuidLabel->setDisabled( true );
    proxyConnUuidEdit->setDisabled( true );

    QObject::connect( connectButton, SIGNAL(clicked(bool)),
                      SLOT(onConnectPressed()) );

    QObject::connect( sendButton, SIGNAL(clicked(bool)),
                      SLOT(onSendPressed()) );

    QObject::connect( useProxyCheckBox, SIGNAL(stateChanged(int)),
                      SLOT(onUseProxyChecked(int)) );

    QObject::connect( securityLevelComboBox, SIGNAL(currentIndexChanged(int)),
                      SLOT(onSecurityLevelChanged(int)) );

    QObject::connect( hostComboBox, SIGNAL(currentIndexChanged(int)),
                      SLOT(onHostComboChanged(int)) );
}

void Server::onConnectPressed ()
{
    if ( m_server == 0  ) {
        m_server = new IOServer(
                       IORoutingTableHelper::GetServerRoutingTable(m_secLevel),
                       IOSender::Client,
                       (hostComboBox->currentIndex() != 2 ?
                          m_host : hostEdit->text()),
                       portSpinBox->value() );

        QObject::connect( m_server,
                      SIGNAL(onPackageReceived(IOSender::Handle,
                                              const SmartPtr<IOPackage>)),
                      SLOT(receivePackage(IOSender::Handle,
                                          const SmartPtr<IOPackage>)) );

        QObject::connect( m_server,
                      SIGNAL(onServerStateChanged(IOSender::State)),
                      SLOT(stateChanged(IOSender::State)) );

        QObject::connect( m_server,
                      SIGNAL(onClientConnected(IOSender::Handle)),
                      SLOT(clientConnected(IOSender::Handle)),
                      Qt::DirectConnection );

        statusBar()->showMessage(tr("Listening ..."));

        m_server->listen();
    }
    else {
        Q_ASSERT(m_server);
        statusBar()->showMessage(tr("Disconnecting ..."));

        m_server->disconnectServer();
    }
}

bool Server::sendTillResult ( const SmartPtr<IOPackage>& p,
                              qint64 nums,
                              quint64& sendLatency )
{
    quint64 localLatency = 0;

    for ( int i = 0; i < nums; ++i ) {

        QList<IOSender::Handle> clients = m_server->getClientsHandles();
        foreach ( IOSender::Handle h, clients ) {
            QApplication::processEvents();

            quint64 msecs = IOService::msecsFromEpoch();
            IOSendJob::Handle job = m_server->sendPackage( h, p );
            IOSendJob::Result res = m_server->waitForSend( job );
            quint64 latency = IOService::msecsFromEpoch() - msecs;
            localLatency += latency;

            if ( res != IOSendJob::Success ) {
                WRITE_TRACE(DBG_FATAL, "Send to client failed: %s", qPrintable(h));
                break;
            }
            res = m_server->getSendResult( job );
            Q_ASSERT( res != IOSendJob::SendQueueIsFull );

            if ( res == IOSendJob::Fail ) {
                WRITE_TRACE(DBG_FATAL, "Send to client failed: %s", qPrintable(h));
                break;
            }
        }

        if ( m_server->state() != IOSender::Connected )
            return false;
    }

    LOG_MESSAGE(DBG_WARNING, "Send pkgs num=%lld of size=%d took: "
                "%lld msecs", nums, p->data[0].bufferSize, localLatency);

    sendLatency += localLatency;

    return true;
}

void Server::onSendPressed ()
{
    // Format:
    // some_data -- sends package with data
    // 1. some_data(NUM) -- sends NUM packages with data
    // 2. some_data(NUMb|m|g) -- sends package with NUM (mega, or giga) bytes
    // 3. some_data(NUMb|m|gXNUM) -- sends NUM packages of NUM (mega, or giga)
    //                                bytes
    // 4. some_data(NUMb|m|g%NUMb|m|g) -- sends packages of NUM (mega, or giga)
    //                                    bytes / NUM (mega, or giga) bytes

    QString str = textEdit->toPlainText();
    textEdit->clear();

    // (.*)\((\d+)\)$
    QRegExp format1( "(.*)\\((\\d+)\\)$",
                     Qt::CaseInsensitive );

    // (.*)\((\d+)(b|m|g{1})\)$
    QRegExp format2( "(.*)\\((\\d+)(b|m|g{1})\\)$",
                     Qt::CaseInsensitive );

    // (.*)\((\d+)(b|m|g{1})x(\d+)\)$
    QRegExp format3( "(.*)\\((\\d+)(b|m|g{1})x(\\d+)\\)$",
                     Qt::CaseInsensitive );

    // (.*)\((\d+)(b|m|g{1})%(\d+)(b|m|g{1})\)$
    QRegExp format4( "(.*)\\((\\d+)(b|m|g{1})%(\\d+)(b|m|g{1})\\)$",
                     Qt::CaseInsensitive );

    quint64 sendLatency = 0;

    if ( format1.indexIn(str) != -1 ) {
        QString data = format1.cap(1);
        qint64 num = format1.cap(2).toInt();

        SmartPtr<IOPackage> p =
            IOPackage::createInstance( 0, 1 );
        p->fillBuffer( 0, IOPackage::RawEncoding,
                       data.toAscii().data(), data.size() );

        sendTillResult( p, num, sendLatency );
    }
    else if ( format2.indexIn(str) != -1 ) {
        QString tmpData = format2.cap(1);
        qint64 num = format2.cap(2).toInt();
        QString type = format2.cap(3);

        // Megs
        if ( type.contains("m", Qt::CaseInsensitive) ) {
            num = 1024 * 1024 * num;
        }
        // Gigs
        else if ( type.contains("g", Qt::CaseInsensitive) ) {
            num = 1024 * 1024 * 1024 * num;
        }

        SmartPtr<IOPackage> p =
            IOPackage::createInstance( 0, 1 );

        QByteArray data( num, '0' );

        int filled = 0;
        int tmpDataSz = tmpData.size();
        QByteArray tmpBa = tmpData.toAscii();
        if ( num > 1024 ) {
            LOG_MESSAGE(DBG_FATAL, "Package size is big, buffer is set with "
                        " zeroes!");
        }
        else {
            while ( filled < num ) {
                if ( (filled + tmpDataSz) > num )
                    tmpDataSz = (filled + tmpDataSz) - num;
                data.replace( filled, tmpDataSz, tmpBa );
                filled += tmpDataSz;
            }
        }

        p->setBuffer( 0, IOPackage::RawEncoding,
                       SmartPtr<char>(data.data(),
                                       SmartPtrPolicy::DoNotReleasePointee),
                       num );

        sendTillResult( p, 1, sendLatency );
    }
    else if ( format3.indexIn(str) != -1 ) {
        QString tmpData = format3.cap(1);
        qint64 num1 = format3.cap(2).toInt();
        QString type = format3.cap(3);
        qint64 num2 = format3.cap(4).toInt();

        // Megs
        if ( type.contains("m", Qt::CaseInsensitive) ) {
            num1 = 1024 * 1024 * num1;
        }
        // Gigs
        else if ( type.contains("g", Qt::CaseInsensitive) ) {
            num1 = 1024 * 1024 * 1024 * num1;
        }

        SmartPtr<IOPackage> p =
            IOPackage::createInstance( 0, 1 );

        QByteArray data( num1, '0' );

        int filled = 0;
        int tmpDataSz = tmpData.size();
        QByteArray tmpBa = tmpData.toAscii();
        if ( num1 > 1024 ) {
            LOG_MESSAGE(DBG_FATAL, "Package size is big, buffer is set with "
                        " zeroes!");
        }
        else {
            while ( filled < num1 ) {
                if ( (filled + tmpDataSz) > num1 )
                    tmpDataSz = (filled + tmpDataSz) - num1;
                data.replace( filled, tmpDataSz, tmpBa );
                filled += tmpDataSz;
            }
        }

        p->setBuffer( 0, IOPackage::RawEncoding,
                       SmartPtr<char>(data.data(),
                                       SmartPtrPolicy::DoNotReleasePointee),
                       num1 );

        sendTillResult( p, num2, sendLatency );
    }
    else if ( format4.indexIn(str) != -1 ) {
        QString tmpData = format4.cap(1);
        qint64 num1 = format4.cap(2).toInt();
        QString type1 = format4.cap(3);
        qint64 num2 = format4.cap(4).toInt();
        QString type2 = format4.cap(5);

        // Megs
        if ( type1.contains("m", Qt::CaseInsensitive) ) {
            num1 = 1024 * 1024 * num1;
        }
        // Gigs
        else if ( type1.contains("g", Qt::CaseInsensitive) ) {
            num1 = 1024 * 1024 * 1024 * num1;
        }

        // Megs
        if ( type2.contains("m", Qt::CaseInsensitive) ) {
            num2 = 1024 * 1024 * num2;
        }
        // Gigs
        else if ( type2.contains("g", Qt::CaseInsensitive) ) {
            num2 = 1024 * 1024 * 1024 * num2;
        }

        SmartPtr<IOPackage> p =
            IOPackage::createInstance( 0, 1 );

        QByteArray data( num2, '0' );


        int filled = 0;
        int tmpDataSz = tmpData.size();
        QByteArray tmpBa = tmpData.toAscii();
        if ( num2 > 1024 ) {
            LOG_MESSAGE(DBG_FATAL, "Package size is big, buffer is set with "
                        " zeroes!");
        }
        else {
            while ( filled < num2 ) {
                if ( (filled + tmpDataSz) > num2 )
                    tmpDataSz = (filled + tmpDataSz) - num2;
                data.replace( filled, tmpDataSz, tmpBa );
                filled += tmpDataSz;
            }
        }

        p->setBuffer( 0, IOPackage::RawEncoding,
                       SmartPtr<char>(data.data(),
                                       SmartPtrPolicy::DoNotReleasePointee),
                       num2 );

        sendTillResult( p, num1 / num2, sendLatency );
    }
    else {
        SmartPtr<IOPackage> p =
            IOPackage::createInstance( 0, 1 );
        p->fillBuffer( 0, IOPackage::RawEncoding,
                       str.toAscii().data(), str.size() );

        sendTillResult( p, 1, sendLatency );
    }
}

void Server::onUseProxyChecked ( int )
{
    bool val = false;
    if ( useProxyCheckBox->isChecked() ) {
        val = false;
    }
    else {
        val = true;
    }

    connUuidLabel->setDisabled( val );
    proxyConnUuidEdit->setDisabled( val );
}

void Server::onSecurityLevelChanged ( int index )
{
    if ( index == 0 )
        m_secLevel = PSL_LOW_SECURITY;
    else if ( index == 1 )
        m_secLevel = PSL_NORMAL_SECURITY;
    else
        m_secLevel = PSL_HIGH_SECURITY;
}

void Server::onHostComboChanged ( int index )
{
    if ( index == 0 ) {
        m_host = IOService::AnyAddr;
        hostEdit->setDisabled( true );
    }
    else if ( index == 1 ) {
        m_host = IOService::LoopbackAddr;
        hostEdit->setDisabled( true );
    }
    else {
        hostEdit->setDisabled( false );
        m_host = hostEdit->text();
    }
}

void Server::receivePackage ( IOSender::Handle h,
                              const SmartPtr<IOPackage> p )
{
    static quint64 pkgNum = 0;
    LOG_MESSAGE( DBG_INFO,"Package from client [host: %s] #%lld",
                 "qPrintable(m_server->clientHostName(h))",
                 ++pkgNum);
    Q_UNUSED(h);

    if ( p->data[0].bufferSize <= 500 ) {
      QString str( QByteArray(p->buffers[0].getImpl(),
                              p->data[0].bufferSize) );
        textEditReceived->append( str );
    } else {
        QString str("PACKAGE #%1 IS TOO BIG (%2 size)");
        textEditReceived->append( str.arg(pkgNum).arg(p->data[0].bufferSize) );
    }
}

void Server::stateChanged ( IOSender::State st )
{
    if ( st == IOSender::Connected ) {
        IOService::IPvBindingInfo info;
        if ( m_server->connectionMode() == IOSender::DirectConnectionMode ) {
            bool res = m_server->serverBindingInfo(info);
            Q_ASSERT(res);
            Q_UNUSED(res);
            WRITE_TRACE(DBG_FATAL, "Server is binded on:");
            QList<IOService::IPVersion> keys = info.keys();
            foreach ( IOService::IPVersion key, keys ) {
                QPair<QString, quint16> bindInfo = info[key];
                WRITE_TRACE(DBG_FATAL, "    %s : %d",
                            qPrintable(bindInfo.first), bindInfo.second );
            }
        }
        else {
            WRITE_TRACE(DBG_FATAL, "Proxy server is connected!");
        }

        connectButton->setText( "Disconnect" );
        statusBar()->showMessage(tr("Listening"));
        sendButton->setDisabled( false );
    }
    else if ( st == IOSender::Disconnected ) {
        connectButton->setText( "Listen" );
        statusBar()->showMessage(tr("Disconnected"));
        sendButton->setDisabled( true );
        delete m_server;
        m_server = 0;
    }
}

void Server::clientConnected ( IOSender::Handle client )
{
    WRITE_TRACE(DBG_FATAL, "New client connected from host: %s",
                qPrintable(m_server->clientHostName(client)));
}

/*****************************************************************************/

int main ( int argc, char* argv[] )
{
    QApplication app( argc, argv );

    Server* ser = new Server;
    ser->show();

    return app.exec();
}

/*****************************************************************************/
