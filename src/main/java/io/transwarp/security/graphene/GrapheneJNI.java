package io.transwarp.security.graphene;

import java.nio.ByteBuffer;

public class GrapheneJNI {
  // mrenclave/mrsigner/isv_prod_id/isv_svn is null if env RA_TLS_MRSIGNER/RA_TLS_MRENCLAVE/RA_TLS_ISV_PROD_ID/RA_TLS_ISV_SVN already configure in manifest
  // otherwise must not null
  public native boolean initClient(String mrenclave, String mrsigner, String isv_prod_id, String isv_svn);
  public native boolean initServer(ByteBuffer key, ByteBuffer crt);
  public native int raTlsVerify(byte[] derCrt);
}