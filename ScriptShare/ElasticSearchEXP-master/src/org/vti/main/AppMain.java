package org.vti.main;

import javax.swing.UIManager;

import org.vti.ui.MainPanel;

public class AppMain {

	public static void main(String[] args) {
		try {
			UIManager.setLookAndFeel("com.sun.java.swing.plaf.nimbus.NimbusLookAndFeel");
			new MainPanel();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}
