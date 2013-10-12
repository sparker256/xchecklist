#import <Cocoa/Cocoa.h>
#import <AppKit/NSSpeechSynthesizer.h>
#import <stdio.h>
#import <unistd.h>
#include "messages.h"


const char cant_alloc_msg[] = "Can't alloc new message!\n";
const char cant_read_msg[] = "Can't read message!\n";

message_t msg;

@interface synt : NSObject<NSSpeechSynthesizerDelegate>
{
    NSSpeechSynthesizer *synth;
    BOOL finishedSpeaking;
    NSString *textToSpeak;
}

-(id)   init;
-(void) dealloc;
-(BOOL) say: (const char*)str;
-(BOOL) speaking;

-(void) speechSynthesizer: (NSSpeechSynthesizer *)sender didFinishSpeaking: (BOOL)success;
/*
-(void) speechSynthesizer: (NSSpeechSynthesizer *)sender didEncounterErrorAtIndex: (NSUInteger)characterIndex 
          ofString:(NSString *)string message:(NSString *)message;
-(void) speechSynthesizer: (NSSpeechSynthesizer *)sender didEncounterSyncMessage:(NSString *)message;
-(void) speechSynthesizer: (NSSpeechSynthesizer *)sender willSpeakPhoneme:(short) phonemeOpcode;
-(void) speechSynthesizer: (NSSpeechSynthesizer *)sender willSpeakWord:(NSRange)wordToSpeak ofString:(NSString *)text;
*/
@end


@implementation synt


-(id) init
{
  if((self = [super init])){
    textToSpeak = nil;
    finishedSpeaking = YES;
    synth = [[NSSpeechSynthesizer alloc] initWithVoice:[NSSpeechSynthesizer defaultVoice]];
    if(synth == nil){
      [self release];
      return nil;
    }
    [synth setDelegate:self];
  }
  return self;
}

-(void) dealloc
{
  [super dealloc];
}

-(BOOL) say: (const char*)str
{
  if(!finishedSpeaking){
    return NO;
  }
  
  finishedSpeaking = NO;
  while([NSSpeechSynthesizer isAnyApplicationSpeaking]){
    sleep(1);
  }
  
  textToSpeak = [[NSString alloc] initWithUTF8String:str];
  [synth startSpeakingString:textToSpeak];
  return YES;
}


-(BOOL) speaking
{
  return !finishedSpeaking;
}

-(void) speechSynthesizer: (NSSpeechSynthesizer *)sender didFinishSpeaking: (BOOL)success
{
  (void) sender;
  (void) success;
  finishedSpeaking = true;
  [textToSpeak release];
  textToSpeak = nil;
  fprintf(stderr, "Finished speaking! %d\n", success);
}

@end

synt *s;



int main(int argc, char *argv[])
{
  (void) argc;
  (void) argv;
  s = [[synt alloc] init];
  if(!new_message(&msg)){
    write(STDOUT_FILENO, cant_alloc_msg, sizeof(cant_alloc_msg));
    return 1;
  }

  while(1){
    if(!read_message(STDIN_FILENO, msg)){
      write(STDOUT_FILENO, cant_read_msg, sizeof(cant_read_msg));
      return 1;
    }
    [s say:str_from_msg(msg)];
    while([s speaking]){
      printf("!");
      [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.2]];
    }
    write(STDOUT_FILENO, "\0", 1);
  }
  [s dealloc];
  
  return 0;
}
/*
  //gcc -o sp -framework AppKit -Wall -Wextra -Werror simon.m && ./sp
*/
