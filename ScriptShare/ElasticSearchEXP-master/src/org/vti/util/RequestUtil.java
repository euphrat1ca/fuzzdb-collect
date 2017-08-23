package org.vti.util;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;

public class RequestUtil {
	
	public String doGetRequest(String uri) throws Exception{
		
		URL url=new URL(uri);
		
		HttpURLConnection conn=(HttpURLConnection) url.openConnection();
		
		conn.setReadTimeout(10*60*1000);
		conn.setReadTimeout(10*60*1000);
		
		BufferedReader reader=new BufferedReader(new InputStreamReader(conn.getInputStream()));
		
		StringBuffer buffer=new StringBuffer();
		String str="";
		while ((str=reader.readLine())!=null) {
			buffer.append(str);
		}
		reader.close();
		return buffer.toString();
	}
	
	public String doPostRequest(String uri,String pram) throws Exception{
		
		URL url=new URL(uri);
		
		HttpURLConnection conn=(HttpURLConnection) url.openConnection();
		
		conn.setRequestMethod("POST");
		conn.setReadTimeout(10*60*1000);
		conn.setReadTimeout(10*60*1000);
		
		conn.setDoOutput(true);
		conn.setDoInput(true);
		
		conn.getOutputStream().write(pram.getBytes());
		conn.getOutputStream().flush();
		conn.getOutputStream().close();
		
		StringBuffer buffer=new StringBuffer();
		
		BufferedReader reader=new BufferedReader( new InputStreamReader(conn.getInputStream()));
		
		String content="";
		
		while ((content=reader.readLine())!=null) {
			buffer.append(content);
		}
		
		return buffer.toString();
	}

}
