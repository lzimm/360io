package com.qorporation {
  
import flash.errors.*;
import flash.events.*;
import flash.net.URLLoader;
import flash.net.URLRequest;
import flash.net.URLVariables;
import flash.net.URLRequestMethod;
import flash.net.URLRequestHeader;
import flash.net.URLVariables;
  
import com.adobe.serialization.json.JSON;
  
public class Request {

    private var m_loader:URLLoader = null;
    private var m_request:URLRequest = null;
    private var m_callback:Function = null;
    
    public function Request(path:String, args:Object, callback:Function):void { 
        m_callback = callback;
               
        m_loader = new URLLoader();
        m_loader.addEventListener(Event.COMPLETE, handleResponse);
        m_loader.addEventListener(IOErrorEvent.IO_ERROR, handleError);
        
        m_request = new URLRequest(path);
        m_request.requestHeaders.push(new URLRequestHeader("pragma", "no-cache"));
        m_request.method = URLRequestMethod.POST;
        
        var variables:URLVariables = new URLVariables();
        for(var arg:String in args) {
            variables[arg] = args[arg];
        }
        
        m_request.data = variables;
    };
    
    public function run():void {
        m_loader.load(m_request);
    };
    
    private function handleResponse(e:Event):void {
        log('handleResponse', e.target.data as String);
        m_callback(e.target.data as String);
        cleanup();
    };
    
    private function handleError(e:IOErrorEvent):void {
        log('handleError', e.toString());
        cleanup();
    };
    
    private function cleanup():void {
        m_loader.close();
    };
    
    private function log(tag:String, msg:String):void {
        trace("[" + tag + "] " + msg + "\n");
    };

};

}