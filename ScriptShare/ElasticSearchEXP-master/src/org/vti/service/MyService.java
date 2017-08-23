package org.vti.service;

import java.util.List;

public interface MyService {
	
	public String doExecuteCMD (String host,String param) throws Exception;
	
	public boolean doUpload (String host,String path,String content) throws Exception;
	
	public List<String> listRoots(String host) throws Exception;

	public List<String> getFiles(String host,String path) throws Exception;
	
	public boolean isDirectory (String host,String path) throws Exception;
	
	public String getFileContent (String host,String path) throws Exception;
	
}
