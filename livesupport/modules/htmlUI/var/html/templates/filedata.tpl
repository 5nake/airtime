{* Smarty template *}
<div id="filedata">
<center>
{if $fMetaData}
    <textarea rows="25" cols="90" style="font-size:small">{htmlspecialchars str=$fMetaData}</textarea>
{/if}
{if $fInfo}
    {$fInfo}
{/if}
</center>



</div>
