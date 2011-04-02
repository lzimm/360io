package com.qorporation {
  
import flash.errors.*;
import flash.events.*;
import flash.net.FileFilter;
import flash.net.FileReference;
import flash.net.URLRequest;
import flash.net.URLVariables;
import flash.net.URLRequestMethod;
import flash.net.URLRequestHeader;
import flash.net.URLVariables;
  
import com.adobe.serialization.json.JSON;
  
public class Upload {

    private var m_id:Number = 0;
    private var m_types:String = null;
    private var m_file:FileReference = null;
    private var m_request:URLRequest = null;
    private var m_callback:Function = null;
    private var m_progress:Function = null;
    private var m_select:Function = null;
    
    public function Upload(id:Number, types:String, select:Function, progress:Function, callback:Function):void { 
        m_id = id;
        m_types = types;
        m_select = select;
        m_progress = progress;
        m_callback = callback;
               
        m_file = new FileReference();
        m_file.addEventListener(DataEvent.UPLOAD_COMPLETE_DATA, handleComplete);
        m_file.addEventListener(IOErrorEvent.IO_ERROR, handleError);
        m_file.addEventListener(ProgressEvent.PROGRESS, handleProgress);
        m_file.addEventListener(Event.SELECT, handleSelect);
    };
    
    public function select():void {
        m_file.browse([new FileFilter("Files", m_types)]);
    };
    
    public function cancel():void {
        m_file.cancel();
    };
    
    public function run(path:String, args:Object):void {
        m_request = new URLRequest(path);
        m_request.requestHeaders.push(new URLRequestHeader("pragma", "no-cache"));
        m_request.method = URLRequestMethod.POST;
        
        var variables:URLVariables = new URLVariables();
        for(var arg:String in args) {
            variables[arg] = args[arg];
        }
        
        m_request.data = variables;
        m_file.upload(m_request, "attachment");
    };
    
    private function handleComplete(e:DataEvent):void {
        log('handleResponse', e.data as String);
        m_callback(m_id, e.data as String);
    };
    
    private function handleError(e:IOErrorEvent):void {
        log('handleError', e.toString());
    };
    
    private function handleProgress(e:ProgressEvent):void {
        log('handleProgress', e.toString());
        m_progress(m_id, e.bytesLoaded, e.bytesTotal);
    };
    
    private function handleSelect(e:Event):void {
        log('handleSelect', e.toString());
        m_select(m_id, m_file.name, m_file.size);
    };
    
    private function log(tag:String, msg:String):void {
        trace("[" + tag + "] " + msg + "\n");
    };

};

}