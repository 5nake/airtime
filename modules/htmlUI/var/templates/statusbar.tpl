{assign var='_nowplaying'  value=$SCHEDULER->getNowNextClip()}
{assign var='_nextplaying' value=$SCHEDULER->getNowNextClip(1)}

{include file="script/clock.js.tpl"}
    <!-- start header -->
        <!-- start header left -->
        <div class="headLeft">
            <img src="img/logo_livesupport.gif" alt="Livesupport Logo" />
            <div class="container">
                {include file="userinfo.tpl"}
            </div>
        </div>
        <!-- end header left -->
        <!-- start header right -->
        <div class="headRight">
            <!-- start station information -->
            <div class="station">
                <img src="{$STATIONPREFS.stationLogoPath}" alt="{$STATIONPREFS.stationName}" width='127' height='34'>
                <div class="frequence">{$STATIONPREFS.stationFrequency}</div>
            </div>
            <!-- end station information -->
            <!-- start onair information -->
            <div class="onair" id="statusbar_indicator">
                {if $_nowplaying}
                    <img src="img/el_onair.gif" alt="on air" />
                {else}
                    <img src="img/el_offair.gif" alt="off air">
                {/if}
            </div>
            <!-- end onair information -->
            <!-- start station time -->
            <div class="stationtime">
                <h1>##Station Time##</h1>
                <div class="time"><span id="statusbar_servertime" class="clock" style="position:relative;">{$smarty.now|date_format:"%H:%M:%S"}</span>
                    <div class="timezone">cet</div>
                </div>
            </div>
            <!-- end station time -->
            <!-- start what playing -->
            <div class="whatplaying" id="statusbar_whatsplaying">
                {if $_nowplaying}
                    <div class="nowplaying">Now Playing: {$_nowplaying.title|truncate:33}</div>
                    <div class="scala">
                        <div class="scala_in" id="statusbar_scala" style="width: {$_nowplaying.percentage}%;">&nbsp;</div>
                    </div>
                    <div class="time">
                        <div class="left">Elapsed:    <strong id="statusbar_elapsed">{$_nowplaying.elapsed.h}:{$_nowplaying.elapsed.m}:{$_nowplaying.elapsed.s|truncate:2:""}</strong></div>
                        <div class="right">Remaining: <strong id="statusbar_remaining">{$_nowplaying.remaining.h}:{$_nowplaying.remaining.m}:{$_nowplaying.remaining.s|truncate:2:""}</strong></div>
                    </div>
                {/if}
                <div class="clearer"></div>
                {if $_nextplaying}
                    <p>Playing Next: {$_nextplaying.title|truncate:22} {$_nextplaying.duration.h}:{$_nextplaying.duration.m}:{$_nextplaying.duration.s|truncate:2:""}</p>
                {/if}
            </div>
            <!-- end what playing -->
        </div>
        <!-- end header right -->
    <!-- end header -->

{assign var='_nowplaying'  value=null}
{assign var='_nextplaying' value=null}
