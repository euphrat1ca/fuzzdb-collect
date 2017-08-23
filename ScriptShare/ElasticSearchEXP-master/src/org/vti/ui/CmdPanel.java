package org.vti.ui;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;

import javax.swing.JPanel;
import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import javax.swing.JTextField;
import javax.swing.LayoutStyle.ComponentPlacement;
import javax.swing.JButton;
import javax.swing.JTextPane;

import org.vti.enumeration.Version;
import org.vti.service.MyService;
import org.vti.service.impl.ESGroovyServiceImpl;
import org.vti.service.impl.ESMvelServiceImp;

public class CmdPanel extends JPanel implements ActionListener{
	 
	private static final long serialVersionUID = 1L;
	
	private JTextField cmdJTextField;
	private JButton exeJButton;
	private JTextPane textPane;
	private String host;
	private Version version;
	
	
	public CmdPanel(){
		setSize(600,460);
		
		JLabel cmdJLabel = new JLabel("CMD");
		
		cmdJTextField = new JTextField("whoami");
		cmdJTextField.setColumns(10);
		
		exeJButton = new JButton("执行");
		exeJButton.addActionListener(this);
		cmdJTextField.addKeyListener(new KeyAdapter() {
			
			@Override
			public void keyPressed(KeyEvent e) {
				super.keyPressed(e);
				if (e.getKeyCode()== KeyEvent.VK_ENTER) {
					
					new Thread(new Runnable() {
						@Override
						public void run() {
							request();
						}
					}).start();
					
				}
			}
		});
		
		textPane = new JTextPane();
		textPane.setEditable(false);
		
		JScrollPane scrollPane=new JScrollPane(textPane);
		
		GroupLayout groupLayout = new GroupLayout(this);
		groupLayout.setHorizontalGroup(
			groupLayout.createParallelGroup(Alignment.LEADING)
				.addGroup(groupLayout.createSequentialGroup()
					.addContainerGap()
					.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
						.addComponent(scrollPane, Alignment.TRAILING, GroupLayout.DEFAULT_SIZE, 580, Short.MAX_VALUE)
						.addGroup(groupLayout.createSequentialGroup()
							.addComponent(cmdJLabel)
							.addPreferredGap(ComponentPlacement.UNRELATED)
							.addComponent(cmdJTextField, GroupLayout.DEFAULT_SIZE, 470, Short.MAX_VALUE)
							.addGap(20)
							.addComponent(exeJButton, GroupLayout.PREFERRED_SIZE, 60, GroupLayout.PREFERRED_SIZE)))
					.addContainerGap())
		);
		groupLayout.setVerticalGroup(
			groupLayout.createParallelGroup(Alignment.LEADING)
				.addGroup(groupLayout.createSequentialGroup()
					.addContainerGap()
					.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
						.addComponent(cmdJLabel)
						.addComponent(cmdJTextField, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(exeJButton))
					.addGap(10)
					.addComponent(scrollPane, GroupLayout.DEFAULT_SIZE, 410, Short.MAX_VALUE)
					.addContainerGap())
		);
		setLayout(groupLayout);
		setVisible(true);
		
		
	}

	@Override
	public void actionPerformed(ActionEvent e) {
	
		if (e.getSource()==exeJButton) {
			new Thread(new Runnable() {
				@Override
				public void run() {
					request();
				}
			}).start();
			
		}
	}
	
	private void request(){
		try {
			textPane.setText("");
			String command=cmdJTextField.getText().trim();
			if (host!=null&&command.length()>0) {
				textPane.setText("请稍候...");
				
				MyService service=null;
				
				if (version.equals(Version.Groovy)) {
					service=new ESGroovyServiceImpl();
				}else {
					service=new ESMvelServiceImp();
				}
				
				textPane.setText(service.doExecuteCMD(host, command));
			
			}else {
				JOptionPane.showMessageDialog(this, "请输入执行命令");
			}
		} catch (Exception exp) {
			textPane.setText(exp.getMessage());
		}
	}
	
	public void setReqestUrl(String host){
		this.host=host;
	}
	
	public void setVersion(Version version){
		this.version=version;
	}
}
