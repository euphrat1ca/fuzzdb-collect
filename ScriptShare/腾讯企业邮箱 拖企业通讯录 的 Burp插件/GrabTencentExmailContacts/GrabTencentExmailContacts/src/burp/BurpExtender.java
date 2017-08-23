package burp;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.io.UnsupportedEncodingException;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * 
 * @author Liar.Bing
 * 
 */
public class BurpExtender implements IBurpExtender, IHttpListener, Runnable {
	//
	// implement IBurpExtender
	//
	private static String TENCENT_MAIL_HOST = "exmail.qq.com";
	private static String MATCH_Path = "/cgi-bin/frame_html";
	private IExtensionHelpers helpers = null;
	private PrintWriter stdout = null;
	private PrintWriter stderr = null;
	private String sid = null;
	private String qm_sid = null;
	private String qm_username = null;
	private IHttpRequestResponse messageInfo = null;

	public void registerExtenderCallbacks(IBurpExtenderCallbacks callbacks) {
		// set our extension name
		callbacks.setExtensionName("Grab tencent exmail contacts");
		helpers = callbacks.getHelpers();
		// obtain our output and error streams
		stdout = new PrintWriter(callbacks.getStdout(), true);
		stderr = new PrintWriter(callbacks.getStderr(), true);

		callbacks.registerHttpListener(this);

	}

	// implement IHttpListener
	public void processHttpMessage(int toolFlag, boolean messageIsRequest,
			IHttpRequestResponse messageInfo) {
		
		if(!messageIsRequest) {
			return;
		}
		IRequestInfo request = null;
		IHttpService httpService = null;
		byte[] request_byte = null;
		this.messageInfo = messageInfo;
		httpService = messageInfo.getHttpService();
		request_byte = messageInfo.getRequest();
		request = this.helpers.analyzeRequest(httpService, request_byte);
		String host = httpService.getHost();
		String path = request.getUrl().getPath();

		// not tencent exmail request
		if (!BurpExtender.TENCENT_MAIL_HOST.equals(host)
				|| !path.contains(BurpExtender.MATCH_Path)) {
			return;
		}
		// request parameters
		List<IParameter> parameters = request.getParameters();
		Map<String, String> paraMap = new HashMap<String, String>();

		// parameter keys
		String key_action = "action";
		String key_sid = "sid";
		String key_t = "t";
		String key_view = "view";

		// put parameter to the hash map
		for (IParameter param : parameters) {
			// stdout.println(param.getName() + " | " + param.getValue());
			if (!param.getName().equals(key_sid)
					|| !paraMap.containsKey(key_sid)) {
				paraMap.put(param.getName(), param.getValue());
			} else if (param.getValue().length() < paraMap.get(key_sid)
					.length()) { // cookie and query string both contain the sid
									// parameter. the shorter one is what we
									// want
				paraMap.put(param.getName(), param.getValue());
			}
		}
		// return;
		// check request parameters
		if (paraMap.containsKey("sid") && paraMap.containsKey("qm_sid")
				&& paraMap.containsKey("qm_username")) {
			this.sid = paraMap.get(key_sid).trim();
			this.qm_sid = paraMap.get("qm_sid").trim();
			this.qm_username = paraMap.get("qm_username").trim();

			if (this.sid != null && !"".equals(this.sid) && this.qm_sid != null
					&& !"".equals(this.qm_sid) && this.qm_username != null
					&& !"".equals(this.qm_username)) {
				new Thread(this).start();
			}
		}

	}

	public String httpGet(String url, String cookie) {
		URL httpURL;
		StringBuilder resultSb = new StringBuilder();
		try {
			httpURL = new URL(url);
			HttpURLConnection http = (HttpURLConnection) httpURL
					.openConnection();
			http.setRequestProperty("Cookie", cookie);

			BufferedReader br = new BufferedReader(new InputStreamReader(
					http.getInputStream(), "gbk"));

			String temp = null;
			while ((temp = br.readLine()) != null) {
				resultSb.append(temp);
				resultSb.append("\n");
			}
		} catch (MalformedURLException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}

		return resultSb.toString();
	}

	public void run() {

		String sid = this.sid;
		String qm_sid = this.qm_sid;
		String qm_username = this.qm_username;
		StringBuffer cookieSb = new StringBuffer();
		cookieSb.append("qm_sid=").append(qm_sid).append(";")
				.append("qm_username=").append(qm_username);

		List<Department> departmentList = new ArrayList<Department>();
		StringBuffer deptSb = new StringBuffer();
		deptSb.append(
				"http://exmail.qq.com/cgi-bin/laddr_biz?action=show_party_list&sid=")
				.append(sid).append("&t=contact&view=biz");

		// get all department info from the response
		Pattern pattern = Pattern
				.compile("\\{id:\"(\\S*?)\", pid:\"(\\S*?)\", name:\"(\\S*?)\", order:\"(\\S*?)\"}");

		String deptRespBody = httpGet(deptSb.toString(), cookieSb.toString());
		Matcher matcher = pattern.matcher(deptRespBody);
		List<Staff> staffList = new ArrayList<Staff>();
		while (matcher.find()) {
			Department department = new Department();
			department.setId(matcher.group(1));
			department.setPid(matcher.group(2));
			department.setName(matcher.group(3));
			department.setOrder(matcher.group(4));
			departmentList.add(department);
			// System.out.println(matcher.group(1));
			// System.out.println(matcher.group(2));
			// System.out.println(matcher.group(3));
			// System.out.println(matcher.group(4));
		}
		Map<String, Department> deptMap = new HashMap<String, Department>();
		Map<String, Staff> staffMap = new HashMap<String, Staff>();
		Department rootDpt = null;
		// fetch all staff
		for (Department department : departmentList) {
			deptMap.put(department.getId(), department);
			if(department.getPid().equals("0")) {
				rootDpt = department;
			}

			StringBuffer sb = new StringBuffer();
			sb.append(
					"http://exmail.qq.com/cgi-bin/laddr_biz?t=memtree&limit=1000&partyid=")
					.append(department.getId())
					.append("&action=show_party&sid=").append(sid);

			String resp = httpGet(sb.toString(), cookieSb.toString());
			System.out.println(resp);
			pattern = Pattern
					.compile("\\{uin:\"(\\S*?)\",pid:\"(\\S*?)\",name:\"(\\S*?)\",alias:\"(\\S*?)\",sex:\"(\\S*?)\",pos:\"(\\S*?)\",tel:\"(\\S*?)\",birth:\"(\\S*?)\",slave_alias:\"(\\S*?)\",department:\"(\\S*?)\",mobile:\"(\\S*?)\"\\}");
			matcher = pattern.matcher(resp.toString());
			while (matcher.find()) {
				Staff staff = new Staff();
				staff.setUin(matcher.group(1));
				staff.setPid(matcher.group(2));
				staff.setName(matcher.group(3));
				staff.setAlias(matcher.group(4));
				staffList.add(staff);
				// stdout.println(matcher.group(3) + "|" + matcher.group(4));
			}
		}
		//stdout.println(departmentList.size());
		
		for (Staff staff : staffList) {
			this.stdout.println(staff.getAlias());
		}
		stdout.println(staffList.size());
		stdout.println("*********************************************");
		stdout.println(departmentList.size());
		print_tree(rootDpt.getId(), deptMap, 0, staffList);
		
	}

	private void print_tree(String id, Map<String, Department> deptMap,
			int level, List<Staff> staff_infors) {
		StringBuffer sb = new StringBuffer();
		StringBuffer prefix = new StringBuffer();
		for (int i = 0; i < level; i++) {
			sb.append("----");
			prefix.append("    ");
		}
		sb.append(" ").append(deptMap.get(id).getName());
		this.stdout.println(sb.toString());

		for (Department dept : deptMap.values()) {
			if (dept.getPid().equals(id)) {
				print_tree(dept.getId(), deptMap, level + 1, staff_infors);
			}
		}

		for (Staff staff : staff_infors) {
			sb = new StringBuffer();
			if (staff.getPid().equals(id)) {
				sb.append(prefix.toString()).append(staff.getName())
						.append("  ").append(staff.getAlias());
				this.stdout.println(sb.toString());
			}
		}
	}

}