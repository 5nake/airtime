{*Smarty template*}

{literal}
<script type="text/javascript">
function form_submit(formname)
{
    var href = '{/literal}{$UI_HANDLER}?act='+formname+'.removeItem{literal}';
    var n;

    for (n=0; n < (document.forms[formname].elements.length-1); n++) {
        if (document.forms[formname].elements[n].checked) {
            href = href + '&id[]=' + document.forms[formname].elements[n].name;
        }
    }   
    hpopup(href);
}

function form_switchAll(formname)
{
    var n;

    for (n=0; n < document.forms[formname].elements.length; n++) {
        if (document.forms[formname].elements[n].type == 'checkbox') {
            document.forms[formname].elements[n].checked = document.forms[formname].elements['all'].checked;
        }
    }
}

function form_clearAll(formname)
{
    if (confirm("{/literal}{tra 0='Are you sure to remove all Items?'}{literal}")) {
        document.forms[formname].elements['all'].checked = true;
        form_switchAll(formname);
        form_submit(formname);
    }
}
</script>
{/literal}
