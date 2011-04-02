package com.qorporation {
  
import flash.errors.*;
import flash.events.*;
import flash.net.URLLoader;
import flash.net.URLRequest;
import flash.net.URLRequestMethod;
import flash.net.URLRequestHeader;
import flash.net.URLVariables;
  
public class Connection {

    private var m_loader:URLLoader = null;
    private var m_request:URLRequest = null;
    private var m_callback:Function = null;
    
    public function Connection(path:String, callback:Function):void {
        m_callback = callback;
        
        m_request = new URLRequest(path);
        m_request.requestHeaders.push(new URLRequestHeader("pragma", "no-cache"));
        m_request.method = URLRequestMethod.POST;
        
        m_loader = new URLLoader();
        m_loader.addEventListener(Event.COMPLETE, handleResponse);
        m_loader.addEventListener(IOErrorEvent.IO_ERROR, handleError);
        
        connect();
    };
    
    public function destroy():void {
        m_loader.close();
    };
    
    private function connect():void {
        m_loader.load(m_request);
    };
    
    private function handleResponse(e:Event):void {
        log('handleResponse', e.target.data as String);
        m_callback(e.target.data as String);
        if (e.target.data != "error") {
            connect();
        } else {
            log('handleResponse', 'received error, disconnecting');
        }
    };
    
    private function handleError(e:IOErrorEvent):void {
        log('handleError', e.toString());
    };
    
    private function log(tag:String, msg:String):void {
        trace("[" + tag + "] " + msg + "\n");
    };

};

}