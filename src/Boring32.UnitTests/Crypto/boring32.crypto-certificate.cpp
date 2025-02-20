#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32.crypto;

namespace Crypto
{
	TEST_CLASS(Certificate)
	{
		public:
			TEST_METHOD(TestDefaultConstructor)
			{
				Boring32::Crypto::Certificate cert;
				Assert::IsNull(cert.GetCert());
			}

			TEST_METHOD(TestCertLoad)
			{
				Boring32::Crypto::CertStore certStore(L"MY");
				Boring32::Crypto::Certificate cert
					= certStore.GetCertBySubjectCn(L"client.localhost");
				Assert::IsNotNull(cert.GetCert());
			}

			TEST_METHOD(TestGetSignatureHashCngAlgorithm)
			{
				Boring32::Crypto::CertStore certStore(L"MY");
				Boring32::Crypto::Certificate cert
					= certStore.GetCertBySubjectCn(L"client.localhost");
				Assert::IsTrue(cert.GetSignatureHashCngAlgorithm() == L"RSA/SHA256");
			}

			TEST_METHOD(TestCopyConstructor)
			{
				Boring32::Crypto::CertStore certStore(L"MY");
				Boring32::Crypto::Certificate cert1 = certStore.GetCertBySubjectCn(L"client.localhost");
				Boring32::Crypto::Certificate cert2(cert1);
				Assert::IsNotNull(cert1.GetCert());
				Assert::IsNotNull(cert2.GetCert());
			}

			TEST_METHOD(TestCopyAssignment)
			{
				Boring32::Crypto::CertStore certStore(L"MY");
				Boring32::Crypto::Certificate cert1 = certStore.GetCertBySubjectCn(L"client.localhost");
				Boring32::Crypto::Certificate cert2 = cert1;
				Assert::IsNotNull(cert1.GetCert());
				Assert::IsNotNull(cert2.GetCert());
			}

			TEST_METHOD(TestMoveConstructor)
			{
				Boring32::Crypto::CertStore certStore(L"MY");
				Boring32::Crypto::Certificate cert1 = certStore.GetCertBySubjectCn(L"client.localhost");
				Boring32::Crypto::Certificate cert2(std::move(cert1));
				Assert::IsNull(cert1.GetCert());
				Assert::IsNotNull(cert2.GetCert());
			}

			TEST_METHOD(TestMoveAssignment)
			{
				Boring32::Crypto::CertStore certStore(L"MY");
				Boring32::Crypto::Certificate cert1 = certStore.GetCertBySubjectCn(L"client.localhost");
				Boring32::Crypto::Certificate cert2 = std::move(cert1);
				Assert::IsNull(cert1.GetCert());
				Assert::IsNotNull(cert2.GetCert());
			}

			TEST_METHOD(TestClose)
			{
				Boring32::Crypto::CertStore certStore(L"MY");
				Boring32::Crypto::Certificate cert1 = certStore.GetCertBySubjectCn(L"client.localhost");
				cert1.Close();
				Assert::IsNull(cert1.GetCert());
			}
	};
}