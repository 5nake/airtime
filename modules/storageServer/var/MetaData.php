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
    Version  : $Revision: 1.14 $
    Location : $Source: /home/paul/cvs2svn-livesupport/newcvsrepo/livesupport/modules/storageServer/var/MetaData.php,v $

------------------------------------------------------------------------------*/

/**
 *  MetaData class
 *
 *  LiveSupport file storage support class.<br>
 *  Store metadata tree in relational database.<br>
 *  <b>requires DOMXML support in PHP!</b>
 *  TODO: use SAX parser instead of DOMXML
 *
 *  @see StoredFile
 */
class MetaData{
    /**
     *  Constructor
     *
     *  @param gb reference to GreenBox object
     *  @param gunid string, global unique id
     *  @param resDir string, resource directory
     *  @return this
     */
    function MetaData(&$gb, $gunid, $resDir)
    {
        $this->dbc        =& $gb->dbc;
        $this->mdataTable = $gb->mdataTable;
        $this->gunid      = $gunid;
        $this->resDir     = $resDir;
        $this->fname      = $this->makeFname();
        $this->exists     =
            $this->dbCheck($gunid) &&
            is_file($this->fname) &&
            is_readable($this->fname)
        ;
    }
    /**
     *  Parse and store metadata from XML file or XML string
     *
     *  @param mdata string, local path to metadata XML file or XML string
     *  @param loc string 'file'|'string'
     *  @return true or PEAR::error
     */
    function insert($mdata, $loc='file')
    {
        if($this->exists) return FALSE;
        $res = $this->storeXMLDoc($mdata, $loc);
        if(PEAR::isError($res)) return $res;
        switch($loc){
        case"file":
            if(! @copy($mdata, $this->fname)){
                return PEAR::raiseError(
                    "MetaData::insert: file save failed".
                    " ($mdata, {$this->fname})",GBERR_FILEIO
                );
            }
            break;
        case"string":
            $fname = $this->fname;
            $e  = FALSE;
            if(!$fh = fopen($fname, "w")){ $e = TRUE; }
            elseif(fwrite($fh, $mdata) === FALSE){ $e = TRUE; }
            if($e){
                return PEAR::raiseError(
                    "BasicStor::bsOpenDownload: can't write ($fname)",
                    GBERR_FILEIO);
            }
            fclose($fh);
            break;
        default:
            return PEAR::raiseError(
                "MetaData::insert: unsupported metadata location ($loc)"
            );
        }
        $this->exists = TRUE;
        return TRUE;
    }
    /**
     *  Parse and update metadata
     *
     *  @param mdata string, local path to metadata XML file or XML string
     *  @param loc string 'file'|'string'
     *  @return true or PEAR::error
     */
    function update($mdata, $loc='file')
    {
        return $this->replace($mdata, $loc);
        /*
        if(!$this->exists) return FALSE;
        $res = $this->storeXMLDoc($mdata, $loc, 'update');
        if(PEAR::isError($res)) return $res;
        $this->exists = TRUE;
        return TRUE;
        */
    }
    /**
     *  Call delete and insert
     *
     *  @param mdata string, local path to metadata XML file or XML string
     *  @param loc string 'file'|'string'
     *  @return true or PEAR::error
     */
    function replace($mdata, $loc='file')
    {
        if($this->exists){
            $res = $this->delete();
            if(PEAR::isError($res)) return $res;
        }
        return $this->insert($mdata, $loc);
    }
    /**
     *  Return true if metadata exists
     *
     *  @return boolean
     */
    function exists()
    {
        return $this->exists;
    }
    /**
     *  Delete all file's metadata
     *
     *  @return true or PEAR::error
     */
    function delete()
    {
        if(file_exists($this->fname)) @unlink($this->fname);
        $res = $this->dbc->query("
            DELETE FROM {$this->mdataTable}
            WHERE gunid=x'{$this->gunid}'::bigint
        ");
        if(PEAR::isError($res)) return $res;
        $this->exists = FALSE;
        return TRUE;
    }
    /**
     *  Return metadata XML string
     *
     *  @return string
     */
    function getMetaData()
    {
        // return $this->genXMLDoc();       // obsolete
        return file_get_contents($this->fname);
    }

    /**
     *  Contruct filepath of metadata file
     *
     *  @return string
     */
    function makeFname()
    {
        return "{$this->resDir}/{$this->gunid}.xml";
    }
    
    /**
     *  Return filename
     *
     *  @return string
     */
    function getFname()
    {
        return $this->fname;
    }
    
    /**
     *  Check if there are any file's metadata in database
     *
     *  @param gunid string, global unique id
     *  @return boolean
     */
    function dbCheck($gunid)
    {
        $cnt = $this->dbc->getOne("
            SELECT count(*)as cnt
            FROM {$this->mdataTable}
            WHERE gunid=x'$gunid'::bigint
        ");
        if(PEAR::isError($cnt)) return $cnt;
        return (intval($cnt) > 0);
    }

    /* ============================================= parse and store metadata */
    /**
     *  Parse and insert or update metadata XML to database
     *
     *  @param mdata string, local path to metadata XML file or XML string
     *  @param loc string 'file'|'string'
     *  @param mode string 'insert'|'update'
     *  @return true or PEAR::error
     */
    function storeXMLDoc($mdata='', $loc='file', $mode='insert')
    {
        switch($loc){
        case"file":
            if(!is_file($mdata)){
                return PEAR::raiseError(
                    "MetaData::storeXMLDoc: metadata file not found ($mdata)"
                );
            }
            if(!is_readable($mdata)){
                return PEAR::raiseError(
                    "MetaData::storeXMLDoc: can't read metadata file ($mdata)"
                );
            }
            $mdstr = file_get_contents($mdata);
            $xml = domxml_open_mem($mdstr);
            break;
        case"string":
            $xml = domxml_open_mem($mdata);
            break;
        default:
            return PEAR::raiseError(
                "MetaData::storeXMLDoc: unsupported metadata location ($loc)"
            );
        }
        $root = $xml->document_element();
        if(!is_object($root)) return PEAR::raiseError(
            "MetaData::storeXMLDoc: metadata parser failed (".gettype($root).")"
        );
        $this->dbc->query("BEGIN");
        if($mode == 'update') $this->nameSpaces = $this->readNamespaces();
        $res = $this->storeXMLNode($root, NULL, $mode);
        if(PEAR::isError($res)){
            $this->dbc->query("ROLLBACK"); return $res;
        }
        foreach($this->nameSpaces as $prefix=>$uri){
            $res = $this->storeRecord(
                '_L', $prefix, NULL, '_namespace', 'T', '_L', $uri, $mode
            );
            if(PEAR::isError($res)){
                $this->dbc->query("ROLLBACK"); return $res;
            }
        }
        $res = $this->dbc->query("COMMIT");
        if(PEAR::isError($res)){ $this->dbc->query("ROLLBACK"); return $res; }
        return TRUE;


        return $root;
    }

    /**
     *  Read namespace definitions from database and return it as array
     *
     *  @return array or PEAR::error
     */
    function readNamespaces()
    {
        $nameSpaces = array();
        $arr = $this->dbc->getAll("SELECT subject, object
            FROM {$this->mdataTable}
            WHERE gunid=x'{$this->gunid}'::bigint
                AND subjns='_L'
                AND predns is null AND predicate='_namespace'
                AND objns='_L'
        ");
        if(PEAR::isError($arr)) return $arr;
        if(is_array($arr)){
            foreach($arr as $i=>$v){
                $nameSpaces[$v['subject']] = $v['object'];
            }
        }
        return $nameSpaces;
    }

    /**
     *  Process one node of metadata XML for insert or update.<br>
     *  <b>TODO: add support for other usable node types</b>
     *
     *  @param node DOM node object
     *  @param parid int, parent id
     *  @param mode 'insert'|'update'
     *  @return
     */
    function storeXMLNode($node, $parid=NULL, $mode='insert')
    {
        //echo $node->node_name().", ".$node->node_type().", ".$node->prefix().", $parid.\n";
        // preprocessing:
        switch($node->node_type()){
            case 1:             // element
            case 2:             // attribute
                $subjns  = (is_null($parid)? '_G'         : '_I');
                $subject = (is_null($parid)? $this->gunid : $parid);
                // DOM XML extension doesn't like empty prefix - use '_d'
                $prefix = $node->prefix(); $prefix = ($prefix === '' ? '_d' : $prefix);
                if(!isset($this->nameSpaces[$prefix]))   
                    $this->nameSpaces[$prefix] = $node->namespace_uri();
            break;
        }
        // main processing:
        switch($node->node_type()){
            case 9:             // document
                $this->storeXMLNode($node->document_element(), $parid, $mode);
            break;
            case 1:             // element
                if($node->is_blank_node()) break;
                $id = $this->storeRecord(
                    $subjns, $subject, $prefix, $node->node_name(), 'T',
                    '_blank', NULL, $mode
                );
                if(PEAR::isError($id)) return $id;
                if($node->has_attributes()){
                    foreach($node->attributes() as $attr){
                        $res = $this->storeXMLNode($attr, $id, $mode);
                        if(PEAR::isError($res)) return $res;
                    }
                }
                if($node->has_child_nodes()){
                    foreach($node->child_nodes() as $child){
                        $res = $this->storeXMLNode($child, $id, $mode);
                        if(PEAR::isError($res)) return $res;
                    }
                }
            break;
            case 2:             // attribute
                $res = $this->storeRecord(
                    $subjns, $subject, $prefix, $node->node_name(),
                    'A', '_L', $node->value(), $mode
                );
                if(PEAR::isError($res)) return $res;
            break;
            case 3:             // text
            case 4:             // cdata
#                echo"T\n";
                if($node->is_blank_node()) break;
                $objns_sql  = "'_L'";
                // coalesce ... returns the first of its arguments that is not NULL
                $object_sql = "coalesce(object,'')||'".$node->node_value()."'";
                $res = $this->dbc->query("
                    UPDATE {$this->mdataTable}
                    SET objns=$objns_sql, object=$object_sql
                    WHERE id='$parid'
                ");
                if(PEAR::isError($res)) return $res;
            break;
            case"5": case"6": case"7": case"8":
            break;
            default:
                return PEAR::raiseError(
                    "MetaData::storeXMLNode: unsupported node type (".
                    $node->node_type().")"
                );
        }
        return TRUE;
    }

    /**
     *  Update object namespace and value of one metadata record
     *  identified by metadata record id
     *
     *  @param mdid int, metadata record id
     *  @param object string, object value, e.g. title string
     *  @param objns string, object namespace prefix, have to be defined
     *          in file's metadata
     *  @return true or PEAR::error
     */
    function updateRecord($mdid, $object, $objns='_L')
    {
        $res = $this->dbc->query("UPDATE {$this->mdataTable}
            SET objns  = '$objns',  object    = '$object'
            WHERE gunid = x'{$this->gunid}'::bigint AND id='$mdid'
        ");
        if(PEAR::isError($res)) return $res;
        return TRUE;
    }

    /**
     *  Insert or update of one metadata record completely
     *
     *  @param subjns string, subject namespace prefix, have to be defined
     *          in file's metadata (or reserved prefix)
     *  @param subject string, subject value, e.g. gunid
     *  @param predns string, predicate namespace prefix, have to be defined
     *          in file's metadata (or reserved prefix)
     *  @param predicate string, predicate value, e.g. name of DC element
     *  @param predxml string 'T'|'A' - XML tag or attribute
     *  @param objns string, object namespace prefix, have to be defined
     *          in file's metadata (or reserved prefix)
     *  @param object string, object value, e.g. title of song
     *  @param mode 'insert'|'update'
     *  @return int, new metadata record id
     */
    function storeRecord($subjns, $subject, $predns, $predicate, $predxml='T',
        $objns=NULL, $object=NULL, $mode='insert')
    {
        //echo "$subjns, $subject, $predns, $predicate, $predxml, $objns, $object, $mode\n";
        $predns_sql = (is_null($predns) ? "NULL" : "'".strtolower($predns)."'" );
        $objns_sql  = (is_null($objns) ? "NULL" : "'$objns'" );
        $object_sql = (is_null($object)? "NULL" : "'$object'");
        $predicate  = strtolower($predicate);
        $id = NULL;
        if($mode == 'update'){
            $cond = "gunid = x'{$this->gunid}'::bigint AND predns=$predns_sql
                AND predicate='$predicate'";
            if($subjns == '_I'){
                $cond .= " AND subjns='_I' AND subject='$subject'";
            }
            $id = $this->dbc->getOne("SELECT id FROM {$this->mdataTable}
                WHERE $cond");
            //echo "$id, ".(is_null($id) ? 'null' : 'not null')."\n";
        }
        if(is_null($id)){ $mode = 'insert'; }
        if($mode == 'insert'){
            $id = $this->dbc->nextId("{$this->mdataTable}_id_seq");
        }
        if(PEAR::isError($id)) return $id;
        if($mode == 'insert'){
            $res = $this->dbc->query("
                INSERT INTO {$this->mdataTable}
                    (id , gunid           , subjns   , subject   ,
                        predns     , predicate   , predxml   ,
                        objns     , object
                    )
                VALUES
                    ($id, x'{$this->gunid}'::bigint, '$subjns', '$subject',
                        $predns_sql, '$predicate', '$predxml',
                        $objns_sql, $object_sql
                    )
            ");
        }else{
            $res = $this->dbc->query("
                UPDATE {$this->mdataTable}
                SET subjns = '$subjns',   subject   = '$subject',
                    objns  = $objns_sql,  object    = $object_sql
                WHERE id='$id'
            ");
            //    WHERE $cond
        }
        if(PEAR::isError($res)) return $res;
        return $id;
    }

    /* =========================================== XML reconstruction from db */
    /**
     *  Generate XML document from metadata database
     *
     *  @return string with XML document
     */
    function genXMLDoc()
    {
        $domd =& domxml_new_xmldoc('1.0');
        $row = $this->dbc->getRow("
            SELECT * FROM {$this->mdataTable}
            WHERE gunid=x'{$this->gunid}'::bigint
                AND subjns='_G' AND subject='{$this->gunid}'
        ");
        if(PEAR::isError($row)) return $row;
        if(is_null($row)){
//            return PEAR::raiseError(
//                "MetaData::genXMLDoc: not exists ({$this->gunid})"
//            );
            $nxn =& $domd->create_element('metadata');
            $domd->append_child($nxn);
        }else{
            $rr = $this->genXMLNode($domd, $domd, $row);
            if(PEAR::isError($rr)) return $rr;
        }
        //return preg_replace("|</([^>]*)>|", "</\\1>\n", $domd->dump_mem())."\n";
        return $domd->dump_mem(TRUE, 'UTF-8');
    }

    /**
     *  Generate XML element from database
     *
     *  @param domd DOM document object
     *  @param xn DOM element object
     *  @param row array, database row with values for created element
     *  @return void
     */
    function genXMLNode(&$domd, &$xn, $row)
    {
        if($row['predxml']=='T'){
            $nxn =& $domd->create_element($row['predicate']);
        }else{
            $nxn =& $domd->create_attribute($row['predicate'], '');
        }
        $xn->append_child($nxn);
        $uri = $this->dbc->getOne("
            SELECT object FROM {$this->mdataTable}
            WHERE gunid=x'{$this->gunid}'::bigint AND predicate='_namespace'
                AND subjns='_L' AND subject='{$row['predns']}'
        ");
        if(!is_null($uri) && $uri !== ''){
            $root =& $domd->document_element();
            if($row['predns'] === '') $row['predns']='_d';
            $root->add_namespace($uri, $row['predns']);
            $nxn->set_namespace($uri, $row['predns']);
        }
        if($row['object'] != 'NULL'){
            $tn =& $domd->create_text_node($row['object']);
            $nxn->append_child($tn);
        }
        $this->genXMLTree($domd, $nxn, $row['id']);
    }

    /**
     *  Generate XML subtree from database
     *
     *  @param domd DOM document object
     *  @param xn DOM element object
     *  @param parid parent id
     *  @return void
     */
    function genXMLTree(&$domd, &$xn, $parid)
    {
        $qh = $this->dbc->query("
            SELECT * FROM {$this->mdataTable}
            WHERE gunid=x'{$this->gunid}'::bigint AND subjns='_I'
                AND subject='$parid'
            ORDER BY id
        ");
        if(PEAR::isError($qh)) return $qh;
        while($row = $qh->fetchRow()){
            $this->genXMLNode($domd, $xn, $row);
        }
        $qh->free();
    }
    
    /* ========================================================= test methods */
    /**
     *  Test method
     *
     *  @return true or PEAR::error
     */
    function test()
    {
        $res = $this->replace(getcwd().'/mdata2.xml');
        if(PEAR::isError($res)) return $res;
        $res = $this->getMetaData();
        if(PEAR::isError($res)) return $res;
        return TRUE;
    }
}
?>
