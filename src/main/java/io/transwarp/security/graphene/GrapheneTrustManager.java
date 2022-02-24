package io.transwarp.security.graphene;

import javax.net.ssl.X509TrustManager;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;

import static io.transwarp.security.ClientDemoApplication.graphene;

public class GrapheneTrustManager implements X509TrustManager {
    @Override
    public void checkClientTrusted(X509Certificate[] x509Certificates, String s) throws CertificateException {
        // do nothing
    }

    @Override
    public void checkServerTrusted(X509Certificate[] x509Certificates, String authType) throws CertificateException {
        if (x509Certificates == null || x509Certificates.length == 0) {
            throw new IllegalArgumentException();
        }
        if (authType == null || authType.length() == 0) {
            throw new IllegalArgumentException();
        }
        if (x509Certificates.length != 1) {
            throw new CertificateException();
        }
        if (graphene.raTlsVerify(x509Certificates[0].getEncoded()) != 0) {
            //System.out.println("server cert not trusted");
            throw new CertificateException();
        }
    }

    @Override
    public X509Certificate[] getAcceptedIssuers() {
        return null;
    }
}
