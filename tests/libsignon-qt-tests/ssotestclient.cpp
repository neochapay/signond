/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
 *
 * Contact: Alberto Mardegan <alberto.mardegan@nokia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include "ssotestclient.h"

#include <QEventLoop>
#include <QTimer>
#include <QTest>
#include <QThread>
#include <QDir>

using namespace SignOn;

int startedClients = 0;
int finishedClients = 0;

/*
 * test timeout 10 seconds
 * */
#define test_timeout 10000
#define pause_time 0

#define TEST_START qDebug("\n\n\n\n ----------------- %s ----------------\n\n",  __func__);

#define TEST_DONE  qDebug("\n\n ----------------- %s PASS ----------------\n\n",  __func__);


#ifdef SSO_TESTS_RUNNING_AS_UNTRUSTED
    #define END_SERVICE_TEST_IF_UNTRUSTED  \
        do {                                                    \
            qDebug() << "\n\nRUNNING UNTRUSTED TEST CLIENT\n\n"; \
            /* todo - remove first condition 1 moth after new err mgmnt release. */ \
            QVERIFY(m_serviceResult.m_error == Error::PermissionDenied);     \
            QVERIFY(m_serviceResult.m_errMsg.contains(SIGNOND_PERMISSION_DENIED_ERR_STR)); \
            TEST_DONE                                                                  \
            return;                                                                    \
        } while(0)

    #define END_IDENTITY_TEST_IF_UNTRUSTED \
        do {                                        \
            qDebug() << "\n\nRUNNING UNTRUSTED TEST CLIENT\n\n"; \
            qDebug() << QString("Code: %1, Msg: %2").arg(errCodeAsStr(m_identityResult.m_error)).arg(m_identityResult.m_errMsg); \
            /* todo - remove first condition 1 moth after new err mgmnt release. */ \
            QVERIFY(m_identityResult.m_error == Error::PermissionDenied);     \
            QVERIFY(m_identityResult.m_errMsg.contains(SIGNOND_PERMISSION_DENIED_ERR_STR));  \
            TEST_DONE                                                                    \
            return;                                                                      \
        } while(0)

    // TODO - define this
    #define END_SESSION_TEST_IF_UNTRUSTED  \
        do {                                                    \
            qDebug() << "\n\nRUNNING UNTRUSTED TEST CLIENT\n\n"; \
            TEST_DONE                                           \
            return;                                             \
        } while(0)
#else
    #define END_SERVICE_TEST_IF_UNTRUSTED  \
        do { qDebug() << "\n\nRUNNING TRUSTED TEST CLIENT\n\n"; } while(0)
    #define END_IDENTITY_TEST_IF_UNTRUSTED \
        do { qDebug() << "\n\nRUNNING TRUSTED TEST CLIENT\n\n"; } while(0)
    #define CHECK_FOR_SESSION_ACCESS_CONTROL_ERROR  \
        do { qDebug() << "\n\nRUNNING TRUSTED TEST CLIENT\n\n"; } while(0)
#endif


#ifdef SSOTESTCLIENT_USES_AUTHSESSION

    #include "testauthsession.h"
    static TestAuthSession testAuthSession;

#endif

//Aegis Tokens for the queryAuthPluginACL test
#define AEGIS_TOKEN_0 "token_0"
#define AEGIS_TOKEN_1 "token_1"
#define AEGIS_TOKEN_2 "libsignon-qt-tests::sso-encryption-token"
#define AEGIS_TOKEN_3 "token_3"
#define AEGIS_TOKEN_4 "token_4"
#define AEGIS_TOKEN_5 "AID::com.nokia.maemo.libsignon-qt-tests.libsignon-qt-tests-id"

SsoTestClient::SsoTestClient()
{
    qDebug() << "PROCESS ID OF TEST CLIENT: " << getpid();
}

void SsoTestClient::runAllTests()
{
    QTime startTime = QTime::currentTime();

    runAuthSessionTests();
    runAuthServiceTests();
    runIdentityTests();

    QTime endTime = QTime::currentTime();
    QTest::qWait(pause_time);

    int elapsed = startTime.secsTo(endTime);
    QTest::qWait(pause_time);

    qDebug() << QString("\n\nTIME --> Elapsed time: %1 seconds\n\n").arg(elapsed);
    emit done();
}

void SsoTestClient::runAuthServiceTests()
{
    qDebug() << "PROCESS ID OF TEST CLIENT: " << getpid();

    initAuthServiceTest();

    QTime startTime = QTime::currentTime();

    queryIdentities();
    queryMethods();
    queryMechanisms();
    clear();

    QTime endTime = QTime::currentTime();

    clearAuthServiceTest();

    int elapsed = startTime.secsTo(endTime);

    qDebug() << QString("\n\nTIME --> Elapsed time: %1 seconds\n\n").arg(elapsed);
    emit done();
}

void SsoTestClient::runIdentityTests()
{
    initIdentityTest();

    QTime startTime = QTime::currentTime();

    queryAvailableMetods();
    storeCredentials();
    requestCredentialsUpdate();
    queryInfo();
    addReference();
    removeReference();
    verifyUser();
    verifySecret();
    signOut();
    remove();
    storeCredentialsWithoutAuthMethodsTest();
    sessionTest();
    multipleRemove();
    removeStoreRemove();

    QTime endTime = QTime::currentTime();QTest::qWait(pause_time);

    clearIdentityTest();
    int elapsed = startTime.secsTo(endTime);

    qDebug() << QString("\n\nTIME --> Elapsed time: %1 seconds\n\n").arg(elapsed);
    emit done();
}

void SsoTestClient::runAuthSessionTests()
{
#ifdef SSOTESTCLIENT_USES_AUTHSESSION
    QTime startTime = QTime::currentTime();

    //run the multi thread test prior to any other thest
    multiThreadTest();

    initAuthSessionTest();
    queryMechanisms_existing_method();
    queryMechanisms_nonexisting_method();
    queryAuthPluginACL();

    process_with_new_identity();
    process_with_existing_identity();
    process_with_nonexisting_type();
    process_with_nonexisting_method();
    process_many_times_after_auth();
    process_many_times_before_auth();
    process_with_big_session_data();

    cancel_immidiately();
    cancel_with_delay();
    cancel_without_process();
    handle_destroyed_signal();

#ifdef SSOUI_TESTS_ENABLED
//    processUi_with_existing_identity();
//    processUi_and_cancel();
#endif

    clearAuthSessionTest();

    QTime endTime = QTime::currentTime();
    int elapsed = startTime.secsTo(endTime);

    qDebug() << QString("\n\nTIME --> Elapsed time: %1 seconds\n\n").arg(elapsed);
    qDebug() << QString("\n\nEnding thread %1.\n\n").arg(QThread::currentThreadId());
    emit done();
#endif
}

QString SsoTestClient::errCodeAsStr(const Error::ErrorType err)
{
    switch(err) {
        case Error::Unknown: return "Unknown";
        case Error::InternalServer: return "InternalServer";
        case Error::InternalCommunication: return "InternalCommunication";
        case Error::PermissionDenied: return "PermissionDenied";
        case Error::MethodNotKnown: return "MethodNotKnown";
        case Error::ServiceNotAvailable: return "ServiceNotAvailable";
        case Error::InvalidQuery: return "InvalidQuery";
        case Error::MethodNotAvailable: return "MethodNotAvailable";
        case Error::IdentityNotFound: return "IdentityNotFound";
        case Error::StoreFailed: return "StoreFailed";
        case Error::RemoveFailed: return "RemoveFailed";
        case Error::SignOutFailed: return "SignOutFailed";
        case Error::IdentityOperationCanceled: return "IdentityOperationCanceled";
        case Error::CredentialsNotAvailable: return "CredentialsNotAvailable";
        case Error::AuthSessionErr: return "AuthSessionErr";
        case Error::MechanismNotAvailable: return "MechanismNotAvailable";
        case Error::MissingData: return "MissingData";
        case Error::InvalidCredentials: return "InvalidCredentials";
        case Error::WrongState: return "WrongState";
        case Error::OperationNotSupported: return "OperationNotSupported";
        case Error::NoConnection: return "NoConnection";
        case Error::Network: return "Network";
        case Error::Ssl: return "Ssl";
        case Error::Runtime: return "Runtime";
        case Error::SessionCanceled: return "SessionCanceled";
        case Error::TimedOut: return "TimedOut";
        case Error::UserInteraction: return "UserInteraction";
        case Error::OperationFailed: return "OperationFailed";
        default: return "DEFAULT error type reached.";
    }
}

bool SsoTestClient::storeCredentialsPrivate(const SignOn::IdentityInfo &info)
{
    Identity *identity = Identity::newIdentity(info, this);

    QEventLoop loop;

    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));

    connect(identity, SIGNAL(credentialsStored(const quint32)),
            &m_identityResult, SLOT(credentialsStored(const quint32)));
    connect(&m_identityResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    identity->storeCredentials();

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    bool ok = false;
    if(m_identityResult.m_responseReceived == TestIdentityResult::NormalResp)
    {
        m_storedIdentityInfo = info;
        m_storedIdentityId = identity->id();
        ok = true;
    }
    else
    {
        qDebug() << "Error reply: " << m_identityResult.m_errMsg
                 << ".\nError code: " << errCodeAsStr(m_identityResult.m_error);
        ok = false;
    }
    delete identity;
    return ok;
}

void SsoTestClient::initIdentityTest()
{
    TEST_START

#ifdef SSO_TESTS_RUNNING_AS_UNTRUSTED
    TEST_DONE
    return;
#endif

    //clearing DB
    AuthService service;
    QEventLoop loop;

    connect(&service, SIGNAL(cleared()), &m_serviceResult, SLOT(cleared()));
    connect(&service, SIGNAL(error(const SignOn::Error &)),
            &m_serviceResult, SLOT(error(const SignOn::Error &)));

    connect(&m_serviceResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    service.clear();

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    if(m_serviceResult.m_responseReceived != TestAuthServiceResult::NormalResp) {
        qDebug() << "Error reply: " << m_serviceResult.m_errMsg
                 << ".\nError code: " << errCodeAsStr(m_serviceResult.m_error);
        QFAIL("Failed to prepare IdentityTest suite.");
    }

    TEST_DONE
}

void SsoTestClient::queryAvailableMetods()
{
    TEST_START

    //inserting some credentials
    QMap<MethodName, MechanismsList> methods;
    methods.insert("method1", QStringList() << "mech1" << "mech2");
    methods.insert("method2", QStringList() << "mech1" << "mech2" << "mech3");
    IdentityInfo info("TEST_CAPTION_1",
                      "TEST_USERNAME_1",
                      methods);
    info.setSecret("TEST_PASSWORD_1");
    info.setAccessControlList(QStringList() << "test_token");

    if(!storeCredentialsPrivate(info))
        QFAIL("Failed to initialize test for querying available methods.");

    Identity *identity = Identity::existingIdentity(m_storedIdentityId, this);

    if(identity == NULL)
        QFAIL("Could not create existing identity. '0' ID provided?");

    m_identityResult.reset();

    QEventLoop loop;

    connect(identity, SIGNAL(methodsAvailable(const QStringList &)),
            &m_identityResult, SLOT(methodsAvailable(const QStringList &)));

    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));

    connect(&m_identityResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    identity->queryAvailableMethods();

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY2(m_identityResult.m_responseReceived != TestIdentityResult::InexistentResp,
             "A response was not received.");

    END_IDENTITY_TEST_IF_UNTRUSTED;

    if(m_identityResult.m_responseReceived == TestIdentityResult::NormalResp)
    {
        qDebug() << "Remote:" << m_identityResult.m_methods;
        qDebug() << "Local:" << m_storedIdentityInfo.methods();

        QVERIFY(m_identityResult.m_methods == m_storedIdentityInfo.methods());
    }
    else
    {
        qDebug() << "Error reply: " << m_identityResult.m_errMsg
                 << ".\nError code: " << errCodeAsStr(m_identityResult.m_error);
        QVERIFY(false);
    }

    TEST_DONE
}

void SsoTestClient::requestCredentialsUpdate()
{
    TEST_START
    QSKIP("This test involves UI interaction...Skipping.", SkipSingle);

    Identity *identity = Identity::existingIdentity(m_storedIdentityId, this);

    if(identity == NULL)
        QFAIL("Could not create existing identity. '0' ID provided?");

    m_identityResult.reset();

    QEventLoop loop;

    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));
    connect(identity, SIGNAL(credentialsStored(const quint32)),
            &m_identityResult, SLOT(credentialsStored(const quint32)));

    connect(&m_identityResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    identity->requestCredentialsUpdate();

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY2(m_identityResult.m_responseReceived != TestIdentityResult::InexistentResp, "A response was not received.");

    END_IDENTITY_TEST_IF_UNTRUSTED;

    if(m_identityResult.m_responseReceived == TestIdentityResult::NormalResp)
    {
        QFAIL("Currently not implemented should have failed.");
    }
    else
    {
        QVERIFY(m_identityResult.m_error == Error::Unknown);
        QCOMPARE(m_identityResult.m_errMsg, QString("Not implemented."));
    }
    TEST_DONE
}

void SsoTestClient::storeCredentials()
{
    TEST_START

    if(!testAddingNewCredentials()) {
        QFAIL("Adding new credentials test failed.");
    }

    if(!testUpdatingCredentials()) {
        END_IDENTITY_TEST_IF_UNTRUSTED;
        QFAIL("Updating existing credentials test failed.");
    }

    TEST_DONE
}

void SsoTestClient::remove()
{
    TEST_START
    m_identityResult.reset();

    //inserting some credentials
    QMap<MethodName, MechanismsList> methods;
    methods.insert("method1", QStringList() << "mech1" << "mech2");
    methods.insert("method2", QStringList() << "mech1" << "mech2" << "mech3");
    IdentityInfo info("TEST_CAPTION_1",
                      "TEST_USERNAME_1",
                      methods);
    info.setSecret("TEST_PASSWORD_1");
    info.setAccessControlList(QStringList() << "test_token");

    if(!storeCredentialsPrivate(info))
        QFAIL("Failed to initialize test for removing identity.");

    Identity *identity = Identity::existingIdentity(m_storedIdentityId, this);
    if(identity == NULL)
        QFAIL("Could not create existing identity. '0' ID provided?");

    QEventLoop loop;

    connect(
            identity,
            SIGNAL(removed()),
            &m_identityResult,
            SLOT(removed()));
    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));

    connect(&m_identityResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    identity->remove();

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY2(m_identityResult.m_responseReceived != TestIdentityResult::InexistentResp,
             "A response was not received.");

    END_IDENTITY_TEST_IF_UNTRUSTED;

    if(m_identityResult.m_responseReceived == TestIdentityResult::NormalResp)
    {
        QVERIFY(m_identityResult.m_removed);
        connect(
            identity,
            SIGNAL(info(const SignOn::IdentityInfo &)),
            &m_identityResult,
            SLOT(info(const SignOn::IdentityInfo &)));

        qDebug() << "Going to query info";
        identity->queryInfo();

        QVERIFY(m_identityResult.m_responseReceived == TestIdentityResult::ErrorResp);
        QVERIFY(m_identityResult.m_error == Error::IdentityNotFound);
    }
    else
    {
        QString codeStr = errCodeAsStr(m_identityResult.m_error);
        qDebug() << "Error reply: " << m_identityResult.m_errMsg
                 << ".\nError code: " << codeStr;

        QFAIL("Should not have received error reply");
    }

    TEST_DONE
}

void SsoTestClient::sessionTest()
{
    TEST_START

    Identity *id = Identity::newIdentity();

    QLatin1String method1Str("mtd1");

    AuthSessionP session1 = id->createSession(method1Str);
    QVERIFY2(session1 != 0, "Could not create auth session.");

    AuthSessionP session2 = id->createSession(method1Str);
    QVERIFY2(session2 == 0, "Multiple auth. sessions with same method created.");

    AuthSessionP session3 = id->createSession("method2");
    QVERIFY2(session3 != 0, "Could not create auth session.");

    id->destroySession(session1);
    session2 = id->createSession(method1Str);
    AuthSessionP session5 = NULL;

    id->destroySession(session5);

    delete id;

    TEST_DONE
}

void SsoTestClient::removeStoreRemove()
{
    TEST_START
    m_identityResult.reset();

    //inserting some credentials
    QMap<MethodName, MechanismsList> methods;
    methods.insert("method1", QStringList() << "mech1" << "mech2");
    methods.insert("method2", QStringList() << "mech1" << "mech2" << "mech3");
    IdentityInfo info("TEST_CAPTION_1",
                      "TEST_USERNAME_1",
                      methods);
    info.setSecret("TEST_PASSWORD_1");
    info.setAccessControlList(QStringList() << "test_token");

    if(!storeCredentialsPrivate(info))
        QFAIL("Failed to initialize test for removing identity.");

    Identity *identity = Identity::existingIdentity(m_storedIdentityId, this);
    if(identity == NULL)
        QFAIL("Could not create existing identity. '0' ID provided?");

    QEventLoop loop;

    connect(
            identity,
            SIGNAL(removed()),
            &m_identityResult,
            SLOT(removed()));
    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));

    connect(&m_identityResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    m_identityResult.reset();
    identity->remove();

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY2(m_identityResult.m_responseReceived != TestIdentityResult::InexistentResp,
             "A response was not received.");

    END_IDENTITY_TEST_IF_UNTRUSTED;

    if(m_identityResult.m_responseReceived == TestIdentityResult::NormalResp)
    {
        QVERIFY(m_identityResult.m_removed);
        m_identityResult.reset();

        qDebug() << "Checking for removed identity ID : " << identity->id();;
        connect(identity, SIGNAL(credentialsStored(const quint32)),
                &m_identityResult, SLOT(credentialsStored(const quint32)));

        QMap<MethodName, MechanismsList> methods;
        methods.insert("method4", QStringList() << "mech10" << "mech20");
        methods.insert("method5", QStringList() << "mech10" << "mech20" << "mech30");
        IdentityInfo updateInfo("TEST_CAPTION_10",
                          "TEST_USERNAME_10",
                          methods);
        updateInfo.setSecret("TEST_PASSWORD_10");
        updateInfo.setAccessControlList(QStringList() << "test_token_1");

        m_identityResult.m_responseReceived = TestIdentityResult::InexistentResp;
        identity->storeCredentials(updateInfo);

        QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
        loop.exec();

        if(m_identityResult.m_responseReceived == TestIdentityResult::NormalResp)
        {
            m_storedIdentityInfo = info;
            m_storedIdentityId = identity->id();
            qDebug() << "Identity ID = " << m_storedIdentityId;
        }
        else
        {
            qDebug() << "Error reply: " << m_identityResult.m_errMsg
                     << ".\nError code: " << errCodeAsStr(m_identityResult.m_error);
            QFAIL("Failed to store the credentials");
        }

        connect(
            identity,
            SIGNAL(info(const SignOn::IdentityInfo &)),
            &m_identityResult,
            SLOT(info(const SignOn::IdentityInfo &)));

        qDebug() << "Checking if the identity info is updated in identity";
        m_identityResult.reset();
        identity->queryInfo();

        QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
        loop.exec();

        QVERIFY(m_identityResult.m_responseReceived == TestIdentityResult::NormalResp);
        if(!TestIdentityResult::compareIdentityInfos(m_identityResult.m_idInfo, updateInfo))
        {
            QFAIL("Compared identity infos are not the same.");
        }

        qDebug() << "Removing after store";
        m_identityResult.reset();
        identity->remove();

        QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
        loop.exec();

        QVERIFY2(m_identityResult.m_responseReceived != TestIdentityResult::InexistentResp,
                 "A response was not received.");
        QVERIFY(m_identityResult.m_removed);

        qDebug() << "Removing again";
        m_identityResult.reset();
        identity->remove();

        QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
        loop.exec();

        QVERIFY(m_identityResult.m_responseReceived == TestIdentityResult::ErrorResp);
        QVERIFY(m_identityResult.m_error == Error::IdentityNotFound);
    }
    else
    {
        QString codeStr = errCodeAsStr(m_identityResult.m_error);
        qDebug() << "Error reply: " << m_identityResult.m_errMsg
                 << ".\nError code: " << codeStr;

        QFAIL("Should not have received error reply");
    }

    delete identity;

    TEST_DONE
}

void SsoTestClient::multipleRemove()
{
    TEST_START
    m_identityResult.reset();

    //inserting some credentials
    QMap<MethodName, MechanismsList> methods;
    methods.insert("method1", QStringList() << "mech1" << "mech2");
    methods.insert("method2", QStringList() << "mech1" << "mech2" << "mech3");
    IdentityInfo info("TEST_CAPTION_1",
                      "TEST_USERNAME_1",
                      methods);
    info.setSecret("TEST_PASSWORD_1");
    info.setAccessControlList(QStringList() << "test_token");

    if(!storeCredentialsPrivate(info))
        QFAIL("Failed to initialize test for removing identity.");

    Identity *identity = Identity::existingIdentity(m_storedIdentityId, this);
    if(identity == NULL)
        QFAIL("Could not create existing identity. '0' ID provided?");

    QEventLoop loop;

    connect(
            identity,
            SIGNAL(removed()),
            &m_identityResult,
            SLOT(removed()));
    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));

    connect(&m_identityResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    identity->remove();

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY2(m_identityResult.m_responseReceived != TestIdentityResult::InexistentResp,
             "A response was not received.");

    END_IDENTITY_TEST_IF_UNTRUSTED;

    if(m_identityResult.m_responseReceived == TestIdentityResult::NormalResp)
    {
        QVERIFY(m_identityResult.m_removed);

        qDebug() << "Removing again";
        m_identityResult.reset();
        identity->remove();

        QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
        loop.exec();

        QVERIFY(m_identityResult.m_responseReceived == TestIdentityResult::ErrorResp);
        QVERIFY(m_identityResult.m_error == Error::IdentityNotFound);
    }
    else
    {
        QString codeStr = errCodeAsStr(m_identityResult.m_error);
        qDebug() << "Error reply: " << m_identityResult.m_errMsg
                 << ".\nError code: " << codeStr;

        QFAIL("Should not have received error reply");
    }

    delete identity;

    TEST_DONE
}

void SsoTestClient::storeCredentialsWithoutAuthMethodsTest()
{
    TEST_START

    if(!testAddingNewCredentials(false)) {
        QFAIL("Adding new credentials test failed.");
    }

    if(!testUpdatingCredentials()) {
        END_IDENTITY_TEST_IF_UNTRUSTED;
        QFAIL("Updating existing credentials test failed.");
    }

    TEST_DONE
}

void SsoTestClient::queryInfo()
{
    TEST_START
    m_identityResult.reset();

    //inserting some credentials
    QMap<MethodName, MechanismsList> methods;
    methods.insert("method1", QStringList() << "mech1" << "mech2");
    methods.insert("method2", QStringList() << "mech1" << "mech2" << "mech3");
    IdentityInfo info("TEST_CAPTION_1",
                      "TEST_USERNAME_1",
                      methods);
    info.setSecret("TEST_PASSWORD_1");
    info.setRealms(QStringList() << "test_realm");
    info.setAccessControlList(QStringList() << "test_token");

    if(!storeCredentialsPrivate(info))
        QFAIL("Failed to initialize test for querying info.");

    Identity *identity = Identity::existingIdentity(m_storedIdentityId);

    if(identity == NULL)
        QFAIL("Could not create existing identity. '0' ID provided?");

    QEventLoop loop;

    connect(
            identity,
            SIGNAL(info(const SignOn::IdentityInfo &)),
            &m_identityResult,
            SLOT(info(const SignOn::IdentityInfo &)));
    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));

    connect(&m_identityResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    identity->queryInfo();

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY2(m_identityResult.m_responseReceived != TestIdentityResult::InexistentResp,
             "A response was not received.");

    END_IDENTITY_TEST_IF_UNTRUSTED;

    if(m_identityResult.m_responseReceived == TestIdentityResult::NormalResp)
    {
        QVERIFY(m_identityResult.m_id == m_storedIdentityId);
        QVERIFY(TestIdentityResult::compareIdentityInfos(
                m_storedIdentityInfo,
                m_identityResult.m_idInfo, false));
    }
    else
    {
        QString codeStr = errCodeAsStr(m_identityResult.m_error);
        qDebug() << "Error reply: " << m_identityResult.m_errMsg
                 << ".\nError code: " << codeStr;

        QFAIL("Should not have received an error reply.");
    }

    TEST_DONE
}

void SsoTestClient::addReference()
{
    TEST_START
    m_identityResult.reset();

    //inserting some credentials
    QMap<MethodName, MechanismsList> methods;
    methods.insert("method1", QStringList() << "mech1" << "mech2");
    methods.insert("method2", QStringList() << "mech1" << "mech2" << "mech3");
    IdentityInfo info("TEST_CAPTION_1",
                      "TEST_USERNAME_1",
                      methods);
    info.setSecret("TEST_PASSWORD_1");
    info.setRealms(QStringList() << "test_realm");
    info.setAccessControlList(QStringList() << "test_token");

    if(!storeCredentialsPrivate(info))
        QFAIL("Failed to initialize test for querying info.");

    Identity *identity = Identity::existingIdentity(m_storedIdentityId);

    if(identity == NULL)
        QFAIL("Could not create existing identity. '0' ID provided?");
    m_identityResult.m_removed = false;
    QEventLoop loop;

    connect(
            identity,
            SIGNAL(referenceAdded()),
            &m_identityResult,
            SLOT(referenceAdded()));
    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));

    connect(&m_identityResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    identity->addReference(QLatin1String("testref"));

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY2(m_identityResult.m_responseReceived != TestIdentityResult::InexistentResp,
             "A response was not received.");

    END_IDENTITY_TEST_IF_UNTRUSTED;

    if(m_identityResult.m_responseReceived == TestIdentityResult::NormalResp)
    {
        QVERIFY(m_identityResult.m_removed);
    }
    else
    {
        QString codeStr = errCodeAsStr(m_identityResult.m_error);
        qDebug() << "Error reply: " << m_identityResult.m_errMsg
                 << ".\nError code: " << codeStr;

        QFAIL("Should not have received an error reply.");
    }

    TEST_DONE
}

void SsoTestClient::removeReference()
{
    m_identityResult.reset();

    //inserting some credentials
    QMap<MethodName, MechanismsList> methods;
    methods.insert("method1", QStringList() << "mech1" << "mech2");
    methods.insert("method2", QStringList() << "mech1" << "mech2" << "mech3");
    IdentityInfo info("TEST_CAPTION_1",
                      "TEST_USERNAME_1",
                      methods);
    info.setSecret("TEST_PASSWORD_1");
    info.setRealms(QStringList() << "test_realm");
    info.setAccessControlList(QStringList() << "test_token");

    if(!storeCredentialsPrivate(info))
        QFAIL("Failed to initialize test for querying info.");

    Identity *identity = Identity::existingIdentity(m_storedIdentityId);

    if(identity == NULL)
        QFAIL("Could not create existing identity. '0' ID provided?");
    m_identityResult.m_removed = false;
    QEventLoop loop;

    identity->addReference(QLatin1String("testref"));

    connect(
            identity,
            SIGNAL(referenceRemoved()),
            &m_identityResult,
            SLOT(referenceRemoved()));
    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));

    connect(&m_identityResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    identity->removeReference(QLatin1String("testref"));

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY2(m_identityResult.m_responseReceived != TestIdentityResult::InexistentResp,
             "A response was not received.");

    END_IDENTITY_TEST_IF_UNTRUSTED;

    if(m_identityResult.m_responseReceived == TestIdentityResult::NormalResp)
    {
        QVERIFY(m_identityResult.m_removed);
    }
    else
    {
        QString codeStr = errCodeAsStr(m_identityResult.m_error);
        qDebug() << "Error reply: " << m_identityResult.m_errMsg
                 << ".\nError code: " << codeStr;

        QFAIL("Should not have received an error reply.");
    }

    TEST_DONE
}

void SsoTestClient::verifyUser()
{
    TEST_START
    m_identityResult.reset();

    //inserting some credentials
    QMap<MethodName, MechanismsList> methods;
    methods.insert("method1", QStringList() << "mech1" << "mech2");
    methods.insert("method2", QStringList() << "mech1" << "mech2" << "mech3");
    IdentityInfo info("TEST_CAPTION_1",
                      "TEST_USERNAME_1",
                      methods);
    info.setSecret("TEST_PASSWORD_1");
    info.setAccessControlList(QStringList() << "test_token");

    if(!storeCredentialsPrivate(info))
        QFAIL("Failed to initialize test for verifying user.");

    Identity *identity = Identity::existingIdentity(m_storedIdentityId);

    if(identity == NULL)
        QFAIL("Could not create existing identity. '0' ID provided?");

    QEventLoop loop;

    connect(
            identity,
            SIGNAL(userVerified(const bool)),
            &m_identityResult,
            SLOT(userVerified(const bool)));
    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));

    connect(&m_identityResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    identity->verifyUser("message");

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY2(m_identityResult.m_responseReceived != TestIdentityResult::InexistentResp,
             "A response was not received.");

    END_IDENTITY_TEST_IF_UNTRUSTED;

    if(m_identityResult.m_responseReceived == TestIdentityResult::NormalResp)
    {
        QFAIL("Currently not implemented should have failed.");
    }
    else
    {
        QCOMPARE(m_identityResult.m_error, Error::Unknown);
        QCOMPARE(m_identityResult.m_errMsg, QString("Not implemented."));
    }

    TEST_DONE
}

void SsoTestClient::verifySecret()
{
    TEST_START
    m_identityResult.reset();

    //inserting some credentials
    QMap<MethodName, MechanismsList> methods;
    methods.insert("method1", QStringList() << "mech1" << "mech2");
    methods.insert("method2", QStringList() << "mech1" << "mech2" << "mech3");
    IdentityInfo info("TEST_CAPTION_1",
                      "TEST_USERNAME_1",
                      methods);
    info.setSecret("TEST_PASSWORD_1");
    info.setAccessControlList(QStringList() << "test_token");

    if(!storeCredentialsPrivate(info))
        QFAIL("Failed to initialize test for verifying secret.");

    Identity *identity = Identity::existingIdentity(m_storedIdentityId);
    if(identity == NULL)
        QFAIL("Could not create existing identity. '0' ID provided?");

    QEventLoop loop;

    connect(
            identity,
            SIGNAL(secretVerified(const bool)),
            &m_identityResult,
            SLOT(secretVerified(const bool)));
    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));

    connect(&m_identityResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    identity->verifySecret("TEST_PASSWORD_1");

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY2(m_identityResult.m_responseReceived != TestIdentityResult::InexistentResp,
             "A response was not received.");

    END_IDENTITY_TEST_IF_UNTRUSTED;

    if(m_identityResult.m_responseReceived == TestIdentityResult::NormalResp)
    {
        QVERIFY(m_identityResult.m_secretVerified);
    }
    else
    {
        QString codeStr = errCodeAsStr(m_identityResult.m_error);
        qDebug() << "Error reply: " << m_identityResult.m_errMsg
                 << ".\nError code: " << codeStr;
        QFAIL("Should not have received an error reply.");
    }

    TEST_DONE
}

void SsoTestClient::signOut()
{
    TEST_START

    m_identityResult.reset();
    //inserting some credentials
    QMap<MethodName, MechanismsList> methods;
    methods.insert("method1", QStringList() << "mech1" << "mech2");
    methods.insert("method2", QStringList() << "mech1" << "mech2" << "mech3");
    IdentityInfo info("TEST_CAPTION_1",
                      "TEST_USERNAME_1",
                      methods);
    info.setSecret("TEST_PASSWORD_1");
    info.setAccessControlList(QStringList() << "test_token");

    if(!storeCredentialsPrivate(info))
        QFAIL("Failed to initialize test for signing out.");

    //create the existing identities
    Identity *identity = Identity::existingIdentity(m_storedIdentityId);
    Identity *identity1 = Identity::existingIdentity(m_storedIdentityId);
    Identity *identity2 = Identity::existingIdentity(m_storedIdentityId);
    Identity *identity3 = Identity::existingIdentity(m_storedIdentityId);

    if(identity == NULL || identity1 == NULL || identity2 == NULL || identity3 == NULL)
        QFAIL("Could not create existing identity. '0' ID provided?");

    connect(
            identity,
            SIGNAL(signedOut()),
            &m_identityResult,
            SLOT(signedOut()));
    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));

    //connect the other 2 identities to designated identity test result objects
    TestIdentityResult identityResult1;
    TestIdentityResult identityResult2;
    TestIdentityResult identityResult3;

    connect(
            identity1,
            SIGNAL(signedOut()),
            &identityResult1,
            SLOT(signedOut()));
    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));

    connect(
            identity2,
            SIGNAL(signedOut()),
            &identityResult2,
            SLOT(signedOut()));
    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));

    connect(
            identity3,
            SIGNAL(signedOut()),
            &identityResult3,
            SLOT(signedOut()));
    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));

    /* Interesting - NOT IN THE GOOD WAY !!!
        - this wait has to be added so that the last identity gets to be registered
          and so that the server can signal it to sign out, too, @TODO
    */
    QTest::qWait(100);

    identity->signOut();

    //this test is likelly to take longer
    QTest::qWait(2000);

    QVERIFY2(m_identityResult.m_responseReceived != TestIdentityResult::InexistentResp,
             "A response was not received.");

    END_IDENTITY_TEST_IF_UNTRUSTED;

    QVERIFY2(identityResult1.m_responseReceived != TestIdentityResult::InexistentResp,
             "A response was not received.");

    QVERIFY2(identityResult2.m_responseReceived != TestIdentityResult::InexistentResp,
             "A response was not received.");

    QVERIFY2(identityResult3.m_responseReceived != TestIdentityResult::InexistentResp,
             "A response was not received.");

    if((m_identityResult.m_responseReceived == TestIdentityResult::NormalResp)
       && (identityResult1.m_responseReceived == TestIdentityResult::NormalResp)
       && (identityResult2.m_responseReceived == TestIdentityResult::NormalResp)
       && (identityResult3.m_responseReceived == TestIdentityResult::NormalResp))
    {
       // probably will do something here in the future
    }
    else
    {
        QString codeStr = errCodeAsStr(m_identityResult.m_error);
        qDebug() << "Error reply: " << m_identityResult.m_errMsg
                 << ".\nError code: " << codeStr;

        QFAIL("Should not have received error reply");
    }

    TEST_DONE
}

void SsoTestClient::clearIdentityTest()
{
    TEST_START
    AuthService *service = new AuthService(this);
    service->clear();
    TEST_DONE
}

void SsoTestClient::initAuthServiceTest()
{
    TEST_START
    //small params preparing
    m_numberOfInsertedCredentials = 5;
    m_expectedNumberOfMethods = (QDir("/usr/lib/signon")).entryList(
            QStringList() << "*.so", QDir::Files).count();
    if(!m_expectedMechanisms.length())
        m_expectedMechanisms << "mech1" << "mech2" << "mech3" << "BLOB";

    m_methodToQueryMechanisms = "ssotest";

#ifdef SSO_TESTS_RUNNING_AS_UNTRUSTED
    TEST_DONE
    return;
#endif

    //clearing DB
    AuthService service;
    QEventLoop loop;

    connect(&service, SIGNAL(cleared()), &m_serviceResult, SLOT(cleared()));
    connect(&service, SIGNAL(error(const SignOn::Error &)),
            &m_serviceResult, SLOT(error(const SignOn::Error &)));
    connect(&m_serviceResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    service.clear();

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    if(m_serviceResult.m_responseReceived != TestAuthServiceResult::NormalResp) {

        QString codeStr = errCodeAsStr(m_serviceResult.m_error);
        qDebug() << "Error reply: " << m_serviceResult.m_errMsg
                 << ".\nError code: " << codeStr;

        QFAIL("Failed to prepare the AuthService test suite.");
    }

    //inserting some credentials
    for(int i = 0; i < m_numberOfInsertedCredentials; i++)
    {
        QMap<MethodName, MechanismsList> methods;
        methods.insert("method1", QStringList() << "mech1" << "mech2");
        methods.insert("method2", QStringList() << "mech1" << "mech2" << "mech3");
        IdentityInfo info(QString("TEST_CAPTION_%1").arg(i),
                          QString("TEST_USERNAME_%1").arg(i),
                          methods);
        info.setSecret(QString("TEST_PASSWORD_%1").arg(i));
        Identity *identity = Identity::newIdentity(info);

        QEventLoop loop;

        connect(identity, SIGNAL(error(const SignOn::Error &)),
                &m_identityResult, SLOT(error(const SignOn::Error &)));
        connect(identity, SIGNAL(credentialsStored(const quint32)),
                &m_identityResult, SLOT(credentialsStored(const quint32)));
        connect(&m_identityResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

        identity->storeCredentials();

        QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
        loop.exec();

        if(m_identityResult.m_responseReceived != TestIdentityResult::NormalResp) {

            QString codeStr = errCodeAsStr(m_identityResult.m_error);
            qDebug() << "Error reply: " << m_identityResult.m_errMsg
                     << ".\nError code: " << codeStr;

            QFAIL("Failed to prepare the AuthService test suite.");
        }

        delete identity;
    }
    TEST_DONE
}

void SsoTestClient::queryMethods()
{
    TEST_START

    m_serviceResult.reset();

    AuthService service;

    QEventLoop loop;

    connect(
            &service,
            SIGNAL(methodsAvailable(const QStringList &)),
            &m_serviceResult,
            SLOT(methodsAvailable(const QStringList &)));
    connect(&service, SIGNAL(error(const SignOn::Error &)),
            &m_serviceResult, SLOT(error(const SignOn::Error &)));

    connect(&m_serviceResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    service.queryMethods();

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY2(m_serviceResult.m_responseReceived != TestAuthServiceResult::InexistentResp,
             "A response was not received.");

    if(m_serviceResult.m_responseReceived == TestAuthServiceResult::NormalResp)
    {
        //this should compare the actual lists not only their count
        QCOMPARE(m_serviceResult.m_methods.count(), m_expectedNumberOfMethods);
    }
    else
    {
        QString codeStr = errCodeAsStr(m_serviceResult.m_error);
        qDebug() << "Error reply: " << m_serviceResult.m_errMsg
                 << ".\nError code: " << codeStr;

        QFAIL("Should not have received error reply");
    }

    TEST_DONE
}

void SsoTestClient::queryMechanisms()
{
    TEST_START
    m_serviceResult.reset();

    AuthService service;

    QEventLoop loop;

    connect(
            &service,
            SIGNAL(mechanismsAvailable(const QString &, const QStringList &)),
            &m_serviceResult,
            SLOT(mechanismsAvailable(const QString &, const QStringList &)));
    connect(&service, SIGNAL(error(const SignOn::Error &)),
            &m_serviceResult, SLOT(error(const SignOn::Error &)));

    connect(&m_serviceResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    service.queryMechanisms(m_methodToQueryMechanisms);

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY2(m_serviceResult.m_responseReceived != TestAuthServiceResult::InexistentResp,
             "A response was not received.");

    if(m_serviceResult.m_responseReceived == TestAuthServiceResult::NormalResp)
    {
        qDebug() << m_serviceResult.m_queriedMechsMethod;
        qDebug() << m_methodToQueryMechanisms;

        qDebug() << m_serviceResult.m_mechanisms.second;
        qDebug() << m_expectedMechanisms;

        QCOMPARE(m_serviceResult.m_queriedMechsMethod, m_methodToQueryMechanisms);
        bool equal = (m_serviceResult.m_mechanisms.second == m_expectedMechanisms);
        QVERIFY(equal);
    }
    else
    {
        QString codeStr = errCodeAsStr(m_serviceResult.m_error);
        qDebug() << "Error reply: " << m_serviceResult.m_errMsg
                 << ".\nError code: " << codeStr;
        QFAIL("Should not have received error reply");
    }

    TEST_DONE
}

void SsoTestClient::queryIdentities()
{
    TEST_START
    m_serviceResult.reset();

    AuthService service;

    QEventLoop loop;

    connect(
            &service,
            SIGNAL(identities(const QList<SignOn::IdentityInfo> &)),
            &m_serviceResult,
            SLOT(identities(const QList<SignOn::IdentityInfo> &)));
    connect(&service, SIGNAL(error(const SignOn::Error &)),
            &m_serviceResult, SLOT(error(const SignOn::Error &)));

    connect(&m_serviceResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    service.queryIdentities();

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY2(m_serviceResult.m_responseReceived != TestAuthServiceResult::InexistentResp,
             "A response was not received.");

    END_SERVICE_TEST_IF_UNTRUSTED;

    if(m_serviceResult.m_responseReceived == TestAuthServiceResult::NormalResp)
    {
        QListIterator<IdentityInfo> it(m_serviceResult.m_identities);
        while(it.hasNext())
        {
            IdentityInfo info = it.next();
            qDebug() << "Identity record: "
                    << "id:" << info.id()
                    << " username: " << info.userName()
                    << " caption: " << info.caption()
                    << " methods:";

            foreach(QString method, info.methods())
                qDebug() << QPair<QString, QStringList>(method, info.mechanisms(method));

        }
        QCOMPARE(m_serviceResult.m_identities.count(), m_numberOfInsertedCredentials);
    }
    else
    {
        QString codeStr = errCodeAsStr(m_serviceResult.m_error);
        qDebug() << "Error reply: " << m_serviceResult.m_errMsg
                 << ".\nError code: " << codeStr;
        QFAIL("Should not have received error reply");
    }

    TEST_DONE
}

void SsoTestClient::queryAuthPluginACL()
{
    TEST_START

    QEventLoop loop;

    //inserting some credentials
    QMap<MethodName, MechanismsList> methods;
    methods.insert("method1", QStringList() << "mech1" << "mech2");
    methods.insert("method2", QStringList() << "mech1" << "mech2" << "mech3");
    IdentityInfo info("test_caption_1",
                      "test_username_1",
                      methods);
    info.setSecret("test_password_1");
    info.setAccessControlList(QStringList() << AEGIS_TOKEN_0 << AEGIS_TOKEN_1 << AEGIS_TOKEN_2
                                            << AEGIS_TOKEN_3 << AEGIS_TOKEN_4 << AEGIS_TOKEN_5);

    Identity *id = Identity::newIdentity(info, this);

    connect(id,
            SIGNAL(credentialsStored(quint32)),
            &m_identityResult,
            SLOT(credentialsStored(quint32)));
    connect(id,
            SIGNAL(error(const SignOn::Error &)),
            &m_identityResult,
            SLOT(error(const SignOn::Error &)));

    connect(&m_identityResult,
            SIGNAL(testCompleted()),
            &loop,
            SLOT(quit()));

    id->storeCredentials();
    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    if (m_identityResult.m_responseReceived != TestIdentityResult::NormalResp) {
        qDebug() << "Error reply: " << m_identityResult.m_errMsg
                 << ".\nError code: " << errCodeAsStr(m_identityResult.m_error);
        QFAIL("Store identity failed.");
    }

    AuthSession *as = id->createSession(QLatin1String("ssotest"));
    connect(as,
            SIGNAL(response(const SignOn::SessionData &)),
            this,
            SLOT(response(const SignOn::SessionData &)));

    QSignalSpy errorSpy(as, SIGNAL(error(const SignOn::Error &)));
    QSignalSpy responseSpy(as, SIGNAL(response(const SignOn::SessionData &)));

    QObject::connect(as,
                     SIGNAL(response(const SignOn::SessionData &)),
                     &loop,
                     SLOT(quit()), Qt::QueuedConnection);
    QObject::connect(as, SIGNAL(error(const SignOn::Error &)), &loop, SLOT(quit()));
    QTimer::singleShot(10*1000, &loop, SLOT(quit()));

    SessionData inData;
    inData.setSecret("testSecret");
    inData.setUserName("testUsername");

    as->process(inData, "mech1");

    if (!errorSpy.count())
        loop.exec();

    QVERIFY(errorSpy.count() == 0);
    QVERIFY(responseSpy.count() == 1);

    QVERIFY(m_tokenList.count() == 2);
    QVERIFY(m_tokenList.contains(AEGIS_TOKEN_2));
    QVERIFY(m_tokenList.contains(AEGIS_TOKEN_5));

    TEST_DONE
}

void SsoTestClient::response(const SignOn::SessionData &data)
{
    m_tokenList = data.getAccessControlTokens();
}

void SsoTestClient::clear()
{
    TEST_START
    m_serviceResult.reset();

    AuthService service;

    QEventLoop loop;

    connect(
            &service,
            SIGNAL(cleared()),
            &m_serviceResult,
            SLOT(cleared()));
    connect(&service, SIGNAL(error(const SignOn::Error &)),
            &m_serviceResult, SLOT(error(const SignOn::Error &)));

    connect(&m_serviceResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    service.clear();

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY2(m_serviceResult.m_responseReceived != TestAuthServiceResult::InexistentResp,
             "A response was not received.");

    END_SERVICE_TEST_IF_UNTRUSTED;

    if(m_serviceResult.m_responseReceived == TestAuthServiceResult::NormalResp) {
        connect(
                &service,
                SIGNAL(identities(const QList<SignOn::IdentityInfo> &)),
                &m_serviceResult,
                SLOT(identities(const QList<SignOn::IdentityInfo> &)));

        service.queryIdentities();

        QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
        loop.exec();
        QVERIFY(m_serviceResult.m_identities.count() == 0);
    } else {
        QString codeStr = errCodeAsStr(m_serviceResult.m_error);
        qDebug() << "Error reply: " << m_serviceResult.m_errMsg
                 << ".\nError code: " << codeStr;

        QFAIL("Should not have received error reply");
    }

    TEST_DONE
}

void SsoTestClient::clearAuthServiceTest()
{
    TEST_START
    TEST_DONE
}

bool SsoTestClient::testAddingNewCredentials(bool addMethods)
{
    m_identityResult.reset();

    QMap<MethodName, MechanismsList> methods;
    if (addMethods) {
        methods.insert("dummy", QStringList() << "mech1" << "mech2" << "mech3");
        methods.insert("dummy1", QStringList() << "mech11" << "mech12" << "mech13");
    }
    IdentityInfo info("TEST_CAPTION", "TEST_USERNAME", methods);
    info.setSecret("TEST_SECRET");
    info.setRealms(QStringList() << "TEST_REALM1" << "TEST_REALM2");

    Identity *identity = Identity::newIdentity(info, this);

    QEventLoop loop;

    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));

    connect(identity, SIGNAL(credentialsStored(const quint32)),
            &m_identityResult, SLOT(credentialsStored(const quint32)));
    connect(&m_identityResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    identity->storeCredentials();

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    if(m_identityResult.m_responseReceived == TestIdentityResult::InexistentResp)
    {
        qDebug() << "A response was not received.";
        return false;
    }

    if(m_identityResult.m_responseReceived == TestIdentityResult::NormalResp)
    {
        if(m_identityResult.m_id != identity->id())
        {
            qDebug() << "Queried identity id does not match with stored data.";
            return false;
        }

        Identity *existingIdentity = Identity::existingIdentity(m_identityResult.m_id, this);
        if(existingIdentity == NULL)
        {
            qDebug() << "Could not create existing identity. '0' ID provided?";
            return false;
        }
        connect(existingIdentity, SIGNAL(info(const SignOn::IdentityInfo &)),
                &m_identityResult, SLOT(info(const SignOn::IdentityInfo &)));

        existingIdentity->queryInfo();

        QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
        loop.exec();
        delete existingIdentity;

        if(!TestIdentityResult::compareIdentityInfos(m_identityResult.m_idInfo, info))
        {
            qDebug() << "Compared identity infos are not the same.";
            return false;
        }
    }
    else
    {
        QString codeStr = errCodeAsStr(m_identityResult.m_error);
        qDebug() << "Error reply: " << m_serviceResult.m_errMsg
                 << ".\nError code: " << codeStr;
        return false;
    }
    return true;
}

bool SsoTestClient::testUpdatingCredentials(bool addMethods)
{
    // Test update credentials functionality

    Identity *existingIdentity = Identity::existingIdentity(m_identityResult.m_id, this);
    if(existingIdentity == NULL)
    {
        qDebug() << "Could not create existing identity. '0' ID provided?";
        return false;
    }

    QMap<MethodName, MechanismsList> methods;
    if (addMethods) {
        methods.insert("dummy1", QStringList() << "mech11" << "mech12" << "mech13");
        methods.insert("dummy2", QStringList() << "mech1_updated" << "mech2" << "mech1_updated2");
        methods.insert("dummy3", QStringList() << "mech1_updated" << "mech2" << "mech1_updated2");
    }

    IdentityInfo updateInfo("TEST_CAPTION", "TEST_USERNAME_UPDATED", methods);
    updateInfo.setSecret("TEST_SECRET_YES", false);

    do
    {
        QEventLoop loop;

        connect(existingIdentity, SIGNAL(error(const SignOn::Error &)),
                &m_identityResult, SLOT(error(const SignOn::Error &)));

        connect(existingIdentity, SIGNAL(credentialsStored(const quint32)),
                &m_identityResult, SLOT(credentialsStored(const quint32)));
        connect(&m_identityResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

        existingIdentity->storeCredentials(updateInfo);
        qDebug();
        QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
        loop.exec();
    } while(0);

    qDebug();
    if(m_identityResult.m_responseReceived == TestIdentityResult::InexistentResp)
    {
        qDebug() << "A response was not received.";
        return false;
    }

    if(m_identityResult.m_responseReceived == TestIdentityResult::NormalResp)
    {
        QEventLoop loop;
        connect(&m_identityResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));
        connect(existingIdentity, SIGNAL(info(const SignOn::IdentityInfo &)),
                &m_identityResult, SLOT(info(const SignOn::IdentityInfo &)));

        existingIdentity->queryInfo();
        QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
        loop.exec();

        qDebug() << "ID:" << existingIdentity->id();
        if(!TestIdentityResult::compareIdentityInfos(m_identityResult.m_idInfo, updateInfo))
        {
            qDebug() << "Compared identity infos are not the same.";
            return false;
        }
    }
    else
    {
        QString codeStr = errCodeAsStr(m_identityResult.m_error);
        qDebug() << "Error reply: " << m_serviceResult.m_errMsg
                 << ".\nError code: " << codeStr;

        return false;
    }
    return true;
}

#ifdef SSOTESTCLIENT_USES_AUTHSESSION

void SsoTestClient::initAuthSessionTest()
{
    TEST_START
    testAuthSession.initTestCase();
    TEST_DONE
}

void SsoTestClient::clearAuthSessionTest()
{
    TEST_START
    testAuthSession.cleanupTestCase();
    TEST_DONE
}

void SsoTestClient::multiThreadTest()
{
    TEST_START
    testAuthSession.multi_thread_test();
    TEST_DONE
}

void SsoTestClient::queryMechanisms_existing_method()
{
    TEST_START
    testAuthSession.queryMechanisms_existing_method();
    TEST_DONE
}

void SsoTestClient::queryMechanisms_nonexisting_method()
{
    TEST_START
    testAuthSession.queryMechanisms_nonexisting_method();
    TEST_DONE
}

void SsoTestClient::process_with_new_identity()
{
    TEST_START
    testAuthSession.process_with_new_identity();
    TEST_DONE
}

void SsoTestClient::process_with_existing_identity()
{
    TEST_START
    testAuthSession.process_with_existing_identity();
    TEST_DONE
}

void SsoTestClient::process_with_nonexisting_type()
{
    TEST_START
    testAuthSession.process_with_nonexisting_type();
    TEST_DONE
}

void SsoTestClient::process_with_nonexisting_method()
{
    TEST_START
    testAuthSession.process_with_nonexisting_method();
    TEST_DONE
}

void SsoTestClient::process_many_times_after_auth()
{
    TEST_START
    testAuthSession.process_many_times_after_auth();
    TEST_DONE
}

void SsoTestClient::process_many_times_before_auth()
{
    TEST_START
    testAuthSession.process_many_times_before_auth();
    TEST_DONE
}

void SsoTestClient::process_with_big_session_data()
{
    TEST_START
    testAuthSession.process_with_big_session_data();
    TEST_DONE
}

void SsoTestClient::cancel_immidiately()
{
    TEST_START
    testAuthSession.cancel_immidiately();
    TEST_DONE
}
void SsoTestClient::cancel_with_delay()
{
    TEST_START
    testAuthSession.cancel_with_delay();
    TEST_DONE
}
void SsoTestClient::cancel_without_process()
{
    TEST_START
    testAuthSession.cancel_without_process();
    TEST_DONE
}

void SsoTestClient::handle_destroyed_signal()
{
    TEST_START
    testAuthSession.handle_destroyed_signal();
    TEST_DONE
}

#ifdef SSOUI_TESTS_ENABLED
void SsoTestClient::processUi_with_existing_identity()
{
    TEST_START
    testAuthSession.processUi_with_existing_identity();
    TEST_DONE
}

void SsoTestClient::processUi_and_cancel()
{
    TEST_START
    testAuthSession.processUi_and_cancel();
    TEST_DONE
}
#endif

#endif