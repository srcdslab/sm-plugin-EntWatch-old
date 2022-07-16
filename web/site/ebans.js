function DoLogin(redir)
{
	var err = 0;
	if(!$('#loginUsername').val())
	{
		$('#loginUsernameMsg').html('You must enter your loginname!');
		$('#loginUsernameMsg').css('display', 'block');
		err++;
	}else
	{
		$('#loginUsernameMsg').html('');
		$('#loginUsernameMsg').css('display', 'none');
	}

	if(!$('#loginPassword').val())
	{
		$('#loginPasswordMsg').html('You must enter your password!');
		$('#loginPasswordMsg').css('display', 'block');
		err++;
	}else
	{
		$('#loginPasswordMsg').html('');
		$('#loginPasswordMsg').css('display', 'none');
	}

	if(err)
		return 0;

	if(redir == "undefined")
		redir = "";

	xajax_Plogin(document.getElementById('loginUsername').value,
				document.getElementById('loginPassword').value,
				 document.getElementById('loginRememberMe').checked,
				 redir);
}

function SaveBan(eban_id, table_name, redir)
{
	if (redir == "undefined")
		redir = "";

	banservername = document.getElementById('banservername').value;
	banplayername = document.getElementById('banplayername').value;
	banplayersteamid = document.getElementById('banplayersteamid').value;
	banreason = document.getElementById('banreason').value;
	banadminname = document.getElementById('banadminname').value;
	banadminsteamid = document.getElementById('banadminsteamid').value;
	banlength = document.getElementById('banlength').value;

	if (banservername == "")
	{
		window.alert("Server name cannot be empty");
		return;
	}
	if (banplayername == "")
	{
		window.alert("Player name cannot be empty");
		return;
	}
	if (banplayersteamid == "")
	{
		window.alert("Player steam id cannot be empty");
		return;
	}
	if (banreason == "")
	{
		window.alert("Ban reason cannot be empty");
		return;
	}
	if (banadminname == "")
	{
		window.alert("Admin name cannot be empty");
		return;
	}
	if (banadminsteamid == "")
	{
		window.alert("Admin steam id cannot be empty");
		return;
	}
	if (banlength == "")
	{
		window.alert("Ban length cannot be empty");
		return;
	}

	if (!window.confirm("Confirm updating eban"))
		return;

	xajax_SaveBan(eban_id,
		table_name,
		banservername,
		banplayername,
		banplayersteamid,
		banreason,
		banadminname,
		banadminsteamid,
		banlength);

}

function DeleteBan(ban_id)
{
	if (!window.confirm("Confirm deleting eban"))
		return;

		xajax_DeleteBan(ban_id);
}

function SetTheme(theme_id)
{
    localStorage.setItem('theme', theme_id);
	document.getElementById('theme').href = "css/themes/" + theme_id + "/main.css";
}

function GetTheme()
{
	let theme_id = localStorage.getItem('theme') || 'black';
	return theme_id;
}

window.addEventListener('load', function () {
	let theme_id = GetTheme();
	SetTheme(theme_id);
})
