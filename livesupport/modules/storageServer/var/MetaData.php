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
    Version  : $Revision: 1.17 $
    Location : $Source: /home/paul/cvs2svn-livesupport/newcvsrepo/livesupport/modules/storageServer/var/MetaData.php,v $

------------------------------------------------------------------------------*/
define('DEBUG', FALSE);
#define('DEBUG', TRUE);
define('MODIFY_LAST_MATCH', TRUE);

require_once "XML/Util.php";

/**
 *  MetaData class
 *
 *  LiveSupport file storage support class.<br>
 *  Store metadata tree in relational database.<br>
 *
 *  @see StoredFile
 *  @see XmlParser
 *  @see DataEngine
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
     *  @param loc string - location: 'file'|'string'
     *  @return true or PEAR::error
     */
    function insert($mdata, $loc='file')
    {
        if($this->exists) return FALSE;
        $res = $this->storeDoc($mdata, $loc);
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
        $res = $this->storeDoc($mdata, $loc, 'update');
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
        if(file_exists($this->fname)){
            $res = file_get_contents($this->fname);
            //require_once "XML/Beautifier.php";
            //$fmt = new XML_Beautifier();
            //$res = $fmt->formatString($res);
            return $res;
        }else
            return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<metadata/>\n";
    }

    /**
     *  Get metadata element value
     *
     *  @param category string, metadata element name
     *  @param lang string, optional xml:lang value for select language version
     *  @param objns string, object namespace prefix - for internal use only
     *  @return array of matching records as hash with fields:
     *   <ul>
     *      <li>mid int, local metadata record id</li>
     *      <li>value string, element value</li>
     *      <li>attrs hasharray of element's attributes indexed by
     *          qualified name (e.g. xml:lang)</li>
     *   </ul>
     *  @see BasicStor::bsGetMetadataValue
     */
    function getMetadataValue($category, $lang=NULL, $objns='_L')
    {
        // handle predicate namespace shortcut
        $a     = XML_Util::splitQualifiedName(strtolower($category));
        if(PEAR::isError($a)) return $a;
        $catNs = $a['namespace'];
        $cat   = $a['localPart'];
        $cond = "
                gunid=x'{$this->gunid}'::bigint AND objns='$objns' AND
                predicate='$cat'
        ";
        if(!is_null($catNs)) $cond .= " AND predns='$catNs'";
        $sql = "
            SELECT id as mid, object as value
            FROM {$this->mdataTable}
            WHERE $cond
            ORDER BY id
        ";
        $all = $this->dbc->getAll($sql);
        if(PEAR::isError($all)) return $all;
        $res = array();
        // add attributes to result
        foreach($all as $i=>$rec){
            $pom = $this->getSubrows($rec['mid']);
            if(PEAR::isError($pom)) return $pom;
            $all[$i]['attrs'] = $pom['attrs'];
            $atlang = $pom['attrs']['xml:lang'];
            // select only matching lang (en is default)
            if(
                is_null($lang) ||
                strtolower($lang) == strtolower($atlang) ||
                (is_null($atlang) && strtolower($lang) == 'en')
            ){
                $res[] = $all[$i];
            }
        }
        return $res;
    }

    /**
     *  Set metadata element value
     *
     *  @param category string, metadata element name (e.g. dc:title)
     *  @param value string/NULL value to store, if NULL then delete record
     *  @param lang string, optional xml:lang value for select language version
     *  @param mid int, metadata record id (OPTIONAL on unique elements)
     *  @param container string, container element name for insert
     *  @return boolean
     */
    function setMetadataValue($category, $value, $lang=NULL, $mid=NULL,
        $container='metadata')
    {
        $rows   = $this->getMetadataValue($category, $lang);
        $aktual = NULL;
        if(count($rows)>1){
            if(is_null($mid)){
                if(MODIFY_LAST_MATCH){ $aktual = array_pop($rows); }
                else{
                    return PEAR::raiseError(
                        "MetaData::setMdataValue:".
                        " nonunique category, mid required"
                    );
                }
            }else{
                foreach($rows as $i=>$row){
                    if($mid == intval($row['mid'])) $aktual = $row;
                }
            }
        }else $aktual = $rows[0];
        if(!is_null($aktual)){
            if(!is_null($value)){
                $sql = "
                    UPDATE {$this->mdataTable}
                    SET object='$value'
                    WHERE id={$aktual['mid']}
                ";
                $res = $this->dbc->query($sql);
            }else{
                $res = $this->deleteRecord($aktual['mid']);
            }
            if(PEAR::isError($res)) return $res;
        }else{
            $contArr = $this->getMetadataValue($container, NULL, '_blank');
            if(PEAR::isError($contArr)) return $contArr;
            $id = $contArr[0]['mid'];
            if(is_null($id)){
                return PEAR::raiseError(
                    "MetaData::setMdataValue: container ($container) not found"
                );
            }
            $a     = XML_Util::splitQualifiedName(strtolower($category));
            if(PEAR::isError($a)) return $a;
            $catNs = $a['namespace'];
            $cat   = $a['localPart'];
            $nid= $this->storeRecord('_I', $id, $catNs, $cat, $predxml='T',
                '_L', $value);
            if(PEAR::isError($nid)) return $nid;
            if(!is_null($lang)){
                $res= $this->storeRecord('_I', $nid, 'xml', 'lang', $predxml='A',
                    '_L', $lang);
                if(PEAR::isError($res)) return $res;
            }
        }
        return TRUE;
    }

    /**
     *  Regenerate XML metadata file after metadata value change
     *
     *  @return boolean
     */
    function regenerateXmlFile()
    {
        $fn = $this->fname;
        $xml = $this->genXMLDoc();
        if (!$fh = fopen($fn, 'w')) {
            return PEAR::raiseError(
                "MetaData::regenerateXmlFile: cannot open for write ($fn)"
            );
        }
        if(fwrite($fh, $xml) === FALSE) {
            return PEAR::raiseError(
                "MetaData::regenerateXmlFile: write error ($fn)"
            );
        }
        fclose($fh);
        return TRUE;
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
     *  @param loc string, location: 'file'|'string'
     *  @return true or PEAR::error
     */
    function storeDoc($mdata='', $loc='file')
    {
        switch($loc){
        case"file":
            if(!is_file($mdata)){
                return PEAR::raiseError(
                    "MetaData::storeDoc: metadata file not found ($mdata)"
                );
            }
            if(!is_readable($mdata)){
                return PEAR::raiseError(
                    "MetaData::storeDoc: can't read metadata file ($mdata)"
                );
            }
            $mdata = file_get_contents($mdata);
        case"string":
            require_once"XmlParser.php";
            $parser =& new XmlParser($mdata);
            if($parser->isError()){
                return PEAR::raiseError(
                    "MetaData::storeDoc: ".$parser->getError()
                );
            }
            $tree = $parser->getTree();
            break;
        default:
            return PEAR::raiseError(
                "MetaData::storeDoc: unsupported metadata location ($loc)"
            );
        }
        $this->dbc->query("BEGIN");
        $res = $this->storeNode($tree);
        if(PEAR::isError($res)){
            $this->dbc->query("ROLLBACK"); return $res;
        }
        $res = $this->dbc->query("COMMIT");
        if(PEAR::isError($res)){ $this->dbc->query("ROLLBACK"); return $res; }
        return TRUE;


        return $root;
    }

    /**
     *  Process one node of metadata XML for insert or update.<br>
     *
     *  @param node object, node in tree returned by XmlParser
     *  @param parid int, parent id
     *  @param nSpaces array of name spaces definitions
     *  @return int, local metadata record id
     */
    function storeNode($node, $parid=NULL, $nSpaces=array())
    {
        //echo $node->node_name().", ".$node->node_type().", ".$node->prefix().", $parid.\n";
        $nSpaces = array_merge($nSpaces, $node->nSpaces);
        // null parid = root node of metadata tree
        $subjns  = (is_null($parid)? '_G'         : '_I');
        $subject = (is_null($parid)? $this->gunid : $parid);
        $object  = $node->content;
        if(is_null($object) || $object == ''){
            $objns  = '_blank';
            $object = 'NULL';
        }else $objns = '_L';
        $id = $this->storeRecord($subjns, $subject,
            $node->ns, $node->name, 'T', $objns, $object);
        // process attributes
        foreach($node->attrs as $atn=>$ato){
            $this->storeRecord('_I', $id,
                $ato->ns, $ato->name, 'A', '_L', $ato->val);
        }
        // process child nodes
        foreach($node->children as $ch){
            $this->storeNode($ch, $id, $nSpaces);
        }
        // process namespace definitions
        foreach($node->nSpaces as $ns=>$uri){
            $this->storeRecord('_I', $id,
                'xmlns', $ns, 'N', '_L', $uri);
        }
        return $id;
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
     *  @param predxml string 'T'|'A'|'N' - XML tag, attribute or NS def.
     *  @param objns string, object namespace prefix, have to be defined
     *          in file's metadata (or reserved prefix)
     *  @param object string, object value, e.g. title of song
     *  @return int, new metadata record id
     */
    function storeRecord($subjns, $subject, $predns, $predicate, $predxml='T',
        $objns=NULL, $object=NULL)
    {
        //echo "$subjns, $subject, $predns, $predicate, $predxml, $objns, $object\n";
        $predns = strtolower($predns);
        $predicate = strtolower($predicate);
        $predns_sql = (is_null($predns) ? "NULL" : "'$predns'" );
        $objns_sql  = (is_null($objns) ? "NULL" : "'$objns'" );
        $object_sql = (is_null($object)? "NULL" : "'$object'");
        $id = $this->dbc->nextId("{$this->mdataTable}_id_seq");
        if(PEAR::isError($id)) return $id;
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
        if(PEAR::isError($res)) return $res;
        return $id;
    }

    /**
     *  Delete metadata record recursively
     *
     *  @param mid int local metadata record id
     *  @return boolean
     */
    function deleteRecord($mid)
    {
        $sql = "
            SELECT id FROM {$this->mdataTable}
            WHERE subjns='_I' AND subject='{$mid}' AND
                gunid=x'{$this->gunid}'::bigint
        ";
        $rh = $this->dbc->query($sql);
        if(PEAR::isError($rh)) return $rh;
        while($row = $rh->fetchRow()){
            $r = $this->deleteRecord($row['id']);
            if(PEAR::isError($r)) return $r;
        }
        $rh->free();
        $sql = "
            DELETE FROM {$this->mdataTable}
            WHERE id={$mid} AND
                gunid=x'{$this->gunid}'::bigint
        ";
        $res = $this->dbc->query($sql);
        if(PEAR::isError($res)) return $res;
        return TRUE;
    }
    
    /* =========================================== XML reconstruction from db */
    /**
     *  Generate XML document from metadata database
     *
     *  @return string with XML document
     */
    function genXMLDoc()
    {
        require_once "XML/Util.php";
        $res = XML_Util::getXMLDeclaration("1.0", "UTF-8")."\n";
        $row = $this->dbc->getRow("
            SELECT * FROM {$this->mdataTable}
            WHERE gunid=x'{$this->gunid}'::bigint
                AND subjns='_G' AND subject='{$this->gunid}'
        ");
        if(PEAR::isError($row)) return $row;
        if(is_null($row)){
            $node = XML_Util::createTagFromArray(array(
                'localpart'=>'none'
            ));
        }else{
            $node = $this->genXMLNode($row);
            if(PEAR::isError($node)) return $node;
        }
        $res .= $node;
        require_once "XML/Beautifier.php";
        $fmt = new XML_Beautifier();
        $res = $fmt->formatString($res);
        return $res;
    }

    /**
     *  Generate XML element from database
     *
     *  @param row array, hash with metadata record fields
     *  @return string, XML serialization of node
     */
    function genXMLNode($row)
    {
        if(DEBUG) echo"genXMLNode:\n";
        if(DEBUG) var_dump($row);
        extract($row);
        $arr = $this->getSubrows($id);
        if(PEAR::isError($arr)) return $arr;
        if(DEBUG) var_dump($arr);
        extract($arr);
        $node = XML_Util::createTagFromArray(array(
            'namespace' => $predns,
            'localPart' => $predicate,
            'attributes'=> $attrs,
#            'content'   => $object." X ".$children,
            'content'   => ($object == 'NULL' ? $children : $object),
        ), FALSE);
        return $node;
    }

    /**
     *  Return values of attributes, child nodes and namespaces for
     *  one metadata record
     *
     *  @param parid int, local id of parent metadata record
     *  @return hash with three fields:
     *      - attr hash, attributes
     *      - children array, child nodes
     *      - nSpaces hash, namespace definitions
     */
    function getSubrows($parid)
    {
        if(DEBUG) echo" getSubrows:\n";
        $qh = $this->dbc->query($q = "
            SELECT
                id, predxml, predns, predicate, objns,
                coalesce(object, 'NULL')as object
            FROM {$this->mdataTable}
            WHERE
                subjns='_I' AND subject='$parid' AND
                gunid=x'{$this->gunid}'::bigint
            ORDER BY id
        ");
        if(PEAR::isError($qh)) return $qh;
        $attrs      = array();
        $children   = array();
        $nSpaces    = array();
        if(DEBUG) echo "  #=".$qh->numRows()."\n$q\n";
        while($row = $qh->fetchRow()){
            if(DEBUG) var_dump($row);
            extract($row);
            switch($predxml){
            case"N":
                $nSpaces["$predicate"] = $object;
            case"A":
                $sep=':';
                if($predns=='' || $predicate=='') $sep='';
                $attrs["{$predns}{$sep}{$predicate}"] = $object;
                break;
            case"T":
                $children[] = $this->genXMLNode($row);
                break;
            default:
                return PEAR::raiseError(
                    "MetaData::getSubrows: unknown predxml ($predxml)");
            } // switch
        }
        $qh->free();
        $children   = join(" ", $children);
        return compact('attrs', 'children', 'nSpaces');
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
