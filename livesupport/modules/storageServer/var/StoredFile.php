<?php
/*------------------------------------------------------------------------------

    Copyright (c) 2004 Media Development Loan Fund
 
    This file is part of the LiveSupport project.
    http://livesupport.campware.org/
    To report bugs, send an e-mail to bugs@campware.org
 
    LiveSupport is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
  
    LiveSupport is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with LiveSupport; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 
 
    Author   : $Author: tomas $
    Version  : $Revision: 1.9 $
    Location : $Source: /home/paul/cvs2svn-livesupport/newcvsrepo/livesupport/modules/storageServer/var/StoredFile.php,v $

------------------------------------------------------------------------------*/
require_once "RawMediaData.php";
require_once "MetaData.php";
require_once "../../../getid3/var/getid3.php";
 
/**
 *  StoredFile class
 *
 *  LiveSupport file storage support class.<br>
 *  Represents one virtual file in storage. Virtual file has up to two parts:
 *  <ul>
 *      <li>metada in database - represeted by MetaData class</li>
 *      <li>binary media data in real file
 *          - represented by RawMediaData class</li>
 *  </ul>
 *  @see GreenBox
 *  @see MetaData
 *  @see RawMediaData
 */
class StoredFile{
    /* ========================================================== constructor */
    /**
     *  Constructor, but shouldn't be externally called
     *
     *  @param gb reference to GreenBox object
     *  @param gunid string, optional, globally unique id of file
     *  @return this
     */
    function StoredFile(&$gb, $gunid=NULL)
    {
        $this->gb         =& $gb;
        $this->dbc        =& $gb->dbc;
        $this->filesTable =  $gb->filesTable;
        $this->accessTable=  $gb->accessTable;
        $this->gunid      =  $gunid;
        if(is_null($this->gunid)) $this->gunid = $this->_createGunid();
        $this->resDir     =  $this->_getResDir($this->gunid);
        $this->accessDir  =  $this->gb->accessDir;
        $this->rmd        =& new RawMediaData($this->gunid, $this->resDir);
        $this->md         =& new MetaData(&$gb, $this->gunid);
        return $this->gunid;
    }

    /* ========= 'factory' methods - should be called to construct StoredFile */
    /**
     *   Create instace of StoreFile object and insert new file
     *
     *  @param gb reference to GreenBox object
     *  @param oid int, local object id in the tree
     *  @param name string, name of new file
     *  @param mediaFileLP string, local path to media file
     *  @param metadata string, local path to metadata XML file or XML string
     *  @param mdataLoc string 'file'|'string' (optional)
     *  @param gunid global unique id (optional) - for insert file with gunid
     *  @return instace of StoredFile object
     */
    function insert(&$gb, $oid, $name,
        $mediaFileLP='', $metadata='', $mdataLoc='file', $gunid=NULL)
    {
        $ac =& new StoredFile(&$gb, ($gunid ? $gunid : NULL));
        $ac->name = $name;
        $ac->id   = $oid;
        $ac->type = "unKnown";
        $emptyState = TRUE;
        if($ac->name=='') $ac->name=$ac->gunid;
        $this->dbc->query("BEGIN");
        $res = $ac->dbc->query("
            INSERT INTO {$ac->filesTable}
                (id, name, gunid, type, state)
            VALUES
                ('$oid', '{$ac->name}', x'{$ac->gunid}'::bigint,
                    '{$ac->type}', 'incomplete')
        ");
        if(PEAR::isError($res)){ $this->dbc->query("ROLLBACK"); return $res; }
        // --- metadata insert:
        if($metadata != ''){
            if($mdataLoc=='file' && !file_exists($metadata))
            {
                return PEAR::raiseError("StoredFile::insert: ".
                    "metadata file doesn't exists ($metadata)");
            }
            $res = $ac->md->insert($metadata, $mdataLoc);
            if(PEAR::isError($res)){
                $this->dbc->query("ROLLBACK"); return $res;
            }
            $emptyState = FALSE;
        }
        // --- media file insert:
        if($mediaFileLP != ''){
            if(!file_exists($mediaFileLP))
            {
                return PEAR::raiseError("StoredFile::insert: ".
                    "media file doesn't exists ($mediaFileLP)");
            }
            $res = $ac->rmd->insert($mediaFileLP);
            if(PEAR::isError($res)){
                $this->dbc->query("ROLLBACK"); return $res;
            }
            $mime = $ac->rmd->getMime();
            //$gb->debugLog("gunid={$ac->gunid}, mime=$mime");
            if($mime !== FALSE){
                $res = $ac->setType($mime);
                if(PEAR::isError($res)){
                    $ac->dbc->query("ROLLBACK"); return $res;
                }
            }
            $emptyState = FALSE;
        }
        if(!$emptyState){
            $res = $ac->setState('ready');
            if(PEAR::isError($res)){ $ac->dbc->query("ROLLBACK"); return $res; }
        }
        $res = $this->dbc->query("COMMIT");
        if(PEAR::isError($res)){ $this->dbc->query("ROLLBACK"); return $res; }
        return $ac;
    }

    /**
     *  Create instace of StoreFile object and recall existing file.<br>
     *  Should be supplied oid XOR gunid - not both ;)
     *
     *  @param gb reference to GreenBox object
     *  @param oid int, optional, local object id in the tree
     *  @param gunid string, optional, global unique id of file
     *  @return instace of StoredFile object
     */
    function recall(&$gb, $oid='', $gunid='')
    {
        $cond = ($oid != ''
            ? "id='".intval($oid)."'"
            : "gunid=x'$gunid'::bigint"
        );
        $row = $gb->dbc->getRow("
            SELECT id, to_hex(gunid)as gunid, type, name
            FROM {$gb->filesTable} WHERE $cond
        ");
        if(PEAR::isError($row)) return $row;
        if(is_null($row)){
            return PEAR::raiseError(
                "StoredFile::recall: fileobj not exist ($oid/$gunid)",
                GBERR_FOBJNEX
            );
        }
        $gunid = StoredFile::_normalizeGunid($row['gunid']);
        $ac =& new StoredFile(&$gb, $gunid);
        $ac->type = $row['type'];
        $ac->name = $row['name'];
        $ac->id   = $row['id'];
        return $ac;
    }

    /**
     *  Create instace of StoreFile object and recall existing file
     *	by gunid.<br/>
     *
     *  @param gb reference to GreenBox object
     *  @param gunid string, optional, global unique id of file
     *  @return instace of StoredFile object
     */
    function recallByGunid(&$gb, $gunid='')
    {
      return StoredFile::recall(&$gb, '', $gunid);
    }

    /**
     *  Create instace of StoreFile object and recall existing file
     *  by access token.<br/>
     *
     *  @param gb reference to GreenBox object
     *  @param token string, access token
     *  @return instace of StoredFile object
     */
    function recallByToken(&$gb, $token)
    {
        $gunid = $gb->dbc->getOne("
            SELECT to_hex(gunid)as gunid
            FROM {$gb->accessTable}
            WHERE token=x'$token'::bigint
        ");
        if(PEAR::isError($gunid)) return $gunid;
        if(is_null($gunid)) return PEAR::raiseError(
            "StoredFile::recallByToken: invalid token ($token)", GBERR_AOBJNEX);
        $gunid = StoredFile::_normalizeGunid($gunid);
        return StoredFile::recall(&$gb, '', $gunid);
    }

    /**
     *  Create instace of StoredFile object and make copy of existing file
     *
     *  @param src reference to source object
     *  @param nid int, new local id
     */
    function copyOf(&$src, $nid)
    {
        $ac =& StoredFile::insert(&$src->gb, $nid, $src->name, $src->_getRealRADFname(), '');
        if(PEAR::isError($ac)) return $ac;
        $ac->md->replace($src->md->getMetaData(), 'xml');
        return $ac;
    }

    /* ======================================================= public methods */
    /**
     *  Replace existing file with new data
     *
     *  @param oid int, local id
     *  @param name string, name of file
     *  @param mediaFileLP string, local path to media file
     *  @param metadata string, local path to metadata XML file or XML string
     *  @param mdataLoc string 'file'|'string'
     */
    function replace($oid, $name, $mediaFileLP='', $metadata='',
        $mdataLoc='file')
    {
        $this->dbc->query("BEGIN");
        $res = $this->rename($name);
        if(PEAR::isError($res)){ $this->dbc->query("ROLLBACK"); return $res; }
        if($mediaFileLP != ''){     // media
            $res = $this->replaceRawMediaData($mediaFileLP);
        }else{
            $res = $this->rmd->delete();
        }
        if(PEAR::isError($res)){
            $this->dbc->query("ROLLBACK"); return $res;
        }
        if($metadata != ''){        // metadata
            $res = $this->md->replace($metadata, $mdataLoc);
        }else{
            $res = $this->md->delete();
        }
        if(PEAR::isError($res)){
            $this->dbc->query("ROLLBACK"); return $res;
        }
        $res = $this->dbc->query("COMMIT");
        if(PEAR::isError($res)){
            $this->dbc->query("ROLLBACK"); return $res;
        }
        return TRUE;
    }

    /**
     *  Increase access counter, create access token, insert access record,
     *  call access method of RawMediaData
     *
     *  @param sessid string
     *  @return array with: access URL, access token
     */
    function accessRawMediaData($sessid)
    {
        $this->dbc->query("BEGIN");
        $res = $this->dbc->query("
            UPDATE {$this->filesTable}
            SET currentlyAccessing=currentlyAccessing+1
            WHERE gunid=x'{$this->gunid}'::bigint
        ");
        if(PEAR::isError($res)){ $this->dbc->query("ROLLBACK"); return $res; }
#        $token = $this->_createGunid();
        $realFname  = $this->_getRealRADFname();
        $ext        = $this->_getExt();

        $res = $this->gb->bsAccess($realFname, $ext, $this->gunid, $sessid);
        if(PEAR::isError($res)){ $this->dbc->query("ROLLBACK"); return $res; }
        $resultArray =
            array('url'=>"file://{$res['fname']}", 'token'=>$res['token']);
        $res = $this->dbc->query("COMMIT");
        if(PEAR::isError($res)){ $this->dbc->query("ROLLBACK"); return $res; }
        return $resultArray;
    }

    /**
     *  Decrease access couter, delete access record,
     *  call release method of RawMediaData
     *
     *  @param token string, access token
     *  @return boolean
     */
    function releaseRawMediaData($token)
    {
        $this->dbc->query("BEGIN");
        $res = $this->dbc->query("UPDATE {$this->filesTable}
            SET currentlyAccessing=currentlyAccessing-1
            WHERE gunid=x'{$this->gunid}'::bigint AND currentlyAccessing>0
        ");
        if(PEAR::isError($res)){ $this->dbc->query("ROLLBACK"); return $res; }
        $res = $this->gb->bsRelease($token);
        if(PEAR::isError($res)){ $this->dbc->query("ROLLBACK"); return $res; }
        $res = $this->dbc->query("COMMIT");
        if(PEAR::isError($res)){ $this->dbc->query("ROLLBACK"); return $res; }
        return TRUE;
    }

    /**
     *  Replace media file only with new binary file
     *
     *  @param mediaFileLP string, local path to media file
     */
    function replaceRawMediaData($mediaFileLP)
    {
        $res = $this->rmd->replace($mediaFileLP);
        if(PEAR::isError($res)){ return $res; }
        $mime = $this->rmd->getMime();
        if($mime !== FALSE){
            $res = $this->setType($mime);
            if(PEAR::isError($res)){ return $res; }
        }
    }

    /**
     *  Replace metadata with new XML file
     *
     *  @param metadata string, local path to metadata XML file or XML string
     *  @param mdataLoc string 'file'|'string'
     *  @return boolean
     */
    function replaceMetaData($metadata, $mdataLoc='file')
    {
        $this->dbc->query("BEGIN");
        $res = $this->md->replace($metadata, $mdataLoc);
        if(PEAR::isError($res)){ $this->dbc->query("ROLLBACK"); return $res; }
        $res = $this->dbc->query("COMMIT");
        if(PEAR::isError($res)) return $res;
        return TRUE;
    }

    /**
     *  Update metadata with new XML file
     *
     *  @param metadata string, local path to metadata XML file or XML string
     *  @param mdataLoc string 'file'|'string'
     *  @return boolean or PEAR::error
     */
    function updateMetaData($metadata, $mdataLoc='file')
    {
        $this->dbc->query("BEGIN");
        $res = $this->md->update($metadata, $mdataLoc);
        if(PEAR::isError($res)){ $this->dbc->query("ROLLBACK"); return $res; }
        $res = $this->dbc->query("COMMIT");
        if(PEAR::isError($res)) return $res;
        return TRUE;
    }

    /**
     *  Update object namespace and value of one metadata record
     *
     *  @param mdid int, metadata record id
     *  @param object string, object value, e.g. title string
     *  @param objns string, object namespace prefix, have to be defined
     *          in file's metadata (or reserved prefix)
     *  @see MetaData
     *  @return boolean or PEAR::error
     */
    function updateMetaDataRecord($mdid, $object, $objns='_L')
    {
        return $this->md->updateRecord($mdid, $object, $objns='_L');
    }

    /**
     *  Get metadata as XML string
     *
     *  @return XML string
     *  @see MetaData
     */
    function getMetaData()
    {
        return $this->md->getMetaData();
    }

    /**
     *  Analyze file with getid3 module.<br>
     *  Obtain some metadata stored in media file.<br>
     *  This method should be used for prefilling metadata input form.
     *
     *  @return array
     *  @see MetaData
     */
    function analyzeMediaFile()
    {
        $ia = $this->rmd->analyze();
        return $ia;
    }

    /**
     *  Rename stored virtual file
     *
     *  @param newname string
     *  @return true or PEAR::error
     */
    function rename($newname)
    {
        $res = $this->dbc->query("
            UPDATE {$this->filesTable} SET name='$newname'
            WHERE gunid=x'{$this->gunid}'::bigint
        ");
        if(PEAR::isError($res)) return $res;
        return TRUE;
    }

    /**
     *  Set state of virtual file
     *
     *  @param state string, 'empty'|'incomplete'|'ready'|'edited'
     *  @return boolean or error
     */
    function setState($state)
    {
        $res = $this->dbc->query("
            UPDATE {$this->filesTable} SET state='$state'
            WHERE gunid=x'{$this->gunid}'::bigint
        ");
        if(PEAR::isError($res)){ return $res; }
        return TRUE;
    }

    /**
     *  Set mime-type of virtual file
     *
     *  @param type string, mime-type
     *  @return boolean or error
     */
    function setType($type)
    {
        $res = $this->dbc->query("
            UPDATE {$this->filesTable} SET type='$type'
            WHERE gunid=x'{$this->gunid}'::bigint
        ");
        if(PEAR::isError($res)){ return $res; }
        return TRUE;
    }

    /**
     *  Delete stored virtual file
     *
     *  @see RawMediaData
     *  @see MetaData
     */
    function delete()
    {
        $res = $this->rmd->delete();
        if(PEAR::isError($res)) return $res;
        $res = $this->md->delete();
        if(PEAR::isError($res)) return $res;
        $tokens = $this->dbc->getAll("
            SELECT to_hex(token)as token, ext FROM {$this->accessTable}
            WHERE gunid=x'{$this->gunid}'::bigint
        ");
        if(is_array($tokens)) foreach($tokens as $i=>$item){
            $file = $this->_getAccessFname($item['token'], $item['ext']);
            if(file_exists($file)){ @unlink($file); }
        }
        $res = $this->dbc->query("
            DELETE FROM {$this->accessTable}
            WHERE gunid=x'{$this->gunid}'::bigint
        ");
        if(PEAR::isError($res)) return $res;
        $res = $this->dbc->query("
            DELETE FROM {$this->filesTable}
            WHERE gunid=x'{$this->gunid}'::bigint
        ");
        if(PEAR::isError($res)) return $res;
        return TRUE;
    }

    /**
     *  Returns true if virtual file is accessed.<br>
     *  Static or dynamic call is possible.
     *
     *  @param gunid string, optional (for static call), global unique id
     */
    function isAccessed($gunid=NULL)
    {
        if(is_null($gunid)) $gunid = $this->gunid;
        return (0 < $this->dbc->getOne("
            SELECT currentlyAccessing FROM {$this->filesTable}
            WHERE gunid=x'$gunid'::bigint
        "));
    }    

    /**
     *  Returns local id of virtual file
     *
     */
    function getId()
    {
        return $this->id;
    }    

    /**
     *  Returns true if raw media file exists
     *
     */
    function exists()
    {
        $indb = $this->dbc->getRow("
            SELECT to_hex(gunid) FROM {$this->filesTable}
            WHERE gunid=x'{$this->gunid}'::bigint
        ");
        return (!is_null($indb) && $this->rmd->exists());
    }    
    
    /* ==================================================== "private" methods */
    /**
     *  Create new global unique id
     *
     */
    function _createGunid()
    {
        $initString =
            microtime().$_SERVER['SERVER_ADDR'].rand()."org.mdlf.livesupport";
        $hash = md5($initString);
        // int8
        $res = substr($hash, 0, 16);
        return StoredFile::_normalizeGunid($res);
    }

    /**
     *  Create new global unique id
     *
     */
    function _normalizeGunid($gunid0)
    {
        return str_pad($gunid0, 16, "0", STR_PAD_LEFT);
    }

    /**
     *  Get local id from global id.
     *  Static or dynamic call is possible.
     *
     *  @param gunid string, optional (for static call),
     *      global unique id of file
     */
    function _idFromGunid($gunid=NULL)
    {
        if(is_null($gunid)) $gunid = $this->$gunid;
        $id = $this->dbc->getOne("
            SELECT id FROM {$this->filesTable}
            WHERE gunid=x'$gunid'::bigint
        ");
        if(is_null($id)) return PEAR::raiseError(
            "StoredFile::_idFromGunid: no such global unique id ($gunid)"
        );
        return $id;
    }

    /**
     *  Return suitable extension.<br>
     *  <b>TODO: make it general - is any tool for it?</b>
     *
     *  @return string file extension without a dot
     */
    function _getExt()
    {
        switch($this->type){
            case"audio/mpeg": $ext="mp3"; break;
            case"audio/x-wave": $ext="wav"; break;
            case"application/x-ogg": $ext="ogg"; break;
            default: $ext="bin"; break;
        }
        return $ext;
    }

    /**
     *  Get filetype from global id
     *
     *  @param gunid string, optional, global unique id of file
     */
    function _getType($gunid)
    {
        return $this->dbc->getOne("
            SELECT type FROM {$this->filesTable}
            WHERE gunid=x'$gunid'::bigint
        ");
    }

    /**
     *  Get and optionaly create subdirectory in real filesystem for storing
     *  raw media data
     *
     */
    function _getResDir()
    {
        $resDir="{$this->gb->storageDir}/".substr($this->gunid, 0, 3);
        // see Transport::_getResDir too for resDir name create code
        if(!file_exists($resDir)){ mkdir($resDir, 02775); chmod($resDir, 02775); }
        return $resDir;
    }

    /**
     *  Get real filename of raw media data
     *
     *  @see RawMediaData
     */
    function _getRealRADFname()
    {
        return $this->rmd->getFname();
    }

    /**
     *  Create and return name for temporary symlink.<br>
     *  <b>TODO: Should be more unique</b>
     *
     */
    function _getAccessFname($token, $ext='EXT')
    {
        return "{$this->accessDir}/$token.$ext";
    }
}
?>