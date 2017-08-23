<?php 
require_once(dirname(__FILE__)."/config.php");
CheckRank(0,0);
$buyid = '';
$ptype = '';
$pname = '';
$price = '';
$buyid = '';
$mtime = time();
$mid = $cfg_ml->M_ID;
$buyid = 'M'.$mid.'T'.$mtime.'RN'.mt_rand(100,999);
$dsql = new DedeSql(false);

//删除用户旧的未付款的同类记录
$mid = intval($mid);
$dsql->ExecuteNoneQuery("Delete From #@__member_operation where mid='$mid' And sta=0 And product='$product'");

$pid = intval($pid);
if($product=='member'){
	$ptype = "会员升级";
	$row = $dsql->GetOne("Select * From #@__member_type where aid='{$pid}'");
	if(!is_array($row)){
		echo "无法识别你的订单！";
		$dsql->Close();
	  exit();
	}
	$pname = $row['pname'];
	$price = $row['money'];
}
else if($product=='card'){
	$ptype = "点卡购买";
	$row = $dsql->GetOne("Select * From #@__moneycard_type where tid='{$pid}'");
	if(!is_array($row)){
		echo "无法识别你的订单！";
		$dsql->Close();
	  exit();
	}
	$pname = $row['pname'];
	$price = $row['money'];
}

if($product=='card'){ $okptype = $ptype.' : 未获得卡号'; }
else{ $okptype = $ptype; }
//保存定单信息

$inquery = "
   INSERT INTO #@__member_operation(`buyid` , `pname` , `product` , `money` , `mtime` , `pid` , `mid` , `sta` ,`oldinfo`) 
   VALUES ('$buyid', '$pname', '$product' , '$price' , '$mtime' , '$pid' , '$mid' , '0' , '$ptype');
";

$isok = $dsql->ExecuteNoneQuery($inquery);

if(!$isok){
  echo "数据库出错，请重新尝试！".$dsql->GetError();
	$dsql->Close();
	exit();
}

if($price==''){
	echo "无法识别你的订单！";
	$dsql->Close();
	exit();
}

$pagePos = 'post_to_pay';
if(empty($cfg_online_type)) $cfg_online_type = 'none';
require_once(dirname(__FILE__).'/config_pay_'.$cfg_online_type.'.php');
require_once(dirname(__FILE__).'/templets/buy_action_'.$cfg_online_type.'.htm');

?>
